WITH RECURSIVE
    offs AS
        (SELECT coalesce(min(ordinal), 0)     as min,
                coalesce(max(ordinal), 0) + 1 as max
         FROM path),
    numbered AS
        (SELECT p.id                                                                                     as id,
                p.name                                                                                   as name,
                p.parent_id                                                                              as parent_id,
                coalesce(p.ordinal, (SELECT max FROM offs)) + (SELECT min FROM offs)                     as ordinal,
                row_number() over (PARTITION BY p.parent_id ORDER BY p.ordinal) + (SELECT max FROM offs) as number
         FROM path p),
    cte(id, qualified_name, ordinal, parent_id, next_parent_id, depth, prep) AS
        (SELECT p.id                      as id,
                p.name                    as qualified_name,
                char(p.ordinal, p.number) as ordinal,
                p.id                      as parent_id,
                p.parent_id               as next_parent_id,
                0                         as depth,
                0                         as prep
         FROM numbered p
         UNION ALL
         SELECT cte.id                                         as id,
                p.name || '.' || cte.qualified_name            as qualified_name,
                CASE
                    WHEN cte.parent_id != cte.next_parent_id
                        THEN char(p.ordinal, p.number)
                    ELSE '' END || cte.ordinal                 as ordinal,
                p.id                                           as parent_id,
                p.parent_id                                    as next_parent_id,
                cte.depth + 1                                  as depth,
                CASE WHEN p.id = p.parent_id THEN 2 ELSE 1 END as prep
         FROM cte
                  LEFT JOIN numbered p on cte.next_parent_id = p.id
         WHERE cte.parent_id != cte.next_parent_id
            or cte.prep = 0),
    dupl AS
        (SELECT cte.qualified_name as qualified_name,
                count(*)           as count
         FROM cte
         GROUP BY cte.qualified_name),
    sub AS
        (SELECT cte.id                           as id,
                cast(cte.qualified_name as text) as qualified_name,
                cte.parent_id                    as parent_id,
                cte.ordinal                      as ordinal,
                cte2.depth - cte.depth           as depth,
                cast(dupl.count == 1 as boolean) as "unique",
                cast(cte.prep == 2 as boolean)   as "full"
         FROM cte
                  LEFT JOIN dupl on dupl.qualified_name = cte.qualified_name
                  LEFT JOIN cte cte2 on cte.id = cte2.id and cte2.prep == 2
         WHERE cte.prep != 1
         ORDER BY cte.prep DESC, cte.ordinal COLLATE binary, cte.depth DESC)
SELECT *
FROM sub
WHERE "full" = true;
