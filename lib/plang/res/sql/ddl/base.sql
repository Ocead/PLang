CREATE TABLE IF NOT EXISTS source
(
    id      integer  not null
        CONSTRAINT source_pk
            PRIMARY KEY autoincrement,
    name    text,
    version text,
    url     text,
    start   datetime not null DEFAULT CURRENT_TIMESTAMP,
    end     datetime not null DEFAULT CURRENT_TIMESTAMP
);

CREATE UNIQUE INDEX IF NOT EXISTS source_id_uindex
    on source (id);

CREATE TABLE IF NOT EXISTS path
(
    id          integer not null
        CONSTRAINT path_pk
            PRIMARY KEY autoincrement,
    name        text    not null
        CHECK (name regexp '^[^.?!()\[\]{\}]*$'),
    parent_id   integer not null
        CONSTRAINT path_path_id_fk
            REFERENCES path
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    ordinal     integer,
    description text,
    source_id   integer
        CONSTRAINT path_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS path_id_uindex
    on path (id);

CREATE UNIQUE INDEX IF NOT EXISTS path_parent_id_name_uindex
    on path (parent_id, name);
