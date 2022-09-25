CREATE TABLE IF NOT EXISTS plot_causal
(
    id        integer not null
        CONSTRAINT plot_causal_pk
            PRIMARY KEY autoincrement,
    op        text    not null
        CHECK ( op in ('*', '+', '~') ),
    source_id integer
        CONSTRAINT plot_causal_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_causal_id_uindex
    on plot_causal (id);

CREATE TABLE IF NOT EXISTS plot_causal_sym
(
    id        integer not null
        CONSTRAINT plot_causal_sym_pk
            PRIMARY KEY autoincrement,
    causal_id integer not null
        CONSTRAINT plot_causal_sym_causal_id_fk
            REFERENCES plot_causal
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    class_id  integer not null
        CONSTRAINT plot_causal_sym_class_id_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id integer
        CONSTRAINT plot_causal_sym_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_causal_sym_id_uindex
    on plot_causal_sym (id);

CREATE TABLE IF NOT EXISTS plot_causal_pnt
(
    id        integer not null
        CONSTRAINT plot_causal_pnt_pk
            PRIMARY KEY autoincrement,
    causal_id integer not null
        CONSTRAINT plot_causal_pnt_causal_id_fk
            REFERENCES plot_causal
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    class_id  integer not null
        CONSTRAINT plot_causal_pnt_class_id_fk
            REFERENCES plot_point_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    direction text    not null
        CHECK ( direction in ('>', '<') ),
    overlap   boolean not null,
    source_id integer
        CONSTRAINT plot_causal_pnt_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_causal_pnt_id_uindex
    on plot_causal_pnt (id);

CREATE TABLE IF NOT EXISTS plot_causal_element_obj
(
    id              integer not null
        CONSTRAINT plot_causal_element_obj_pk
            PRIMARY KEY autoincrement,
    causal_id       integer not null
        CONSTRAINT plot_causal_element_obj_causal_id_fk
            REFERENCES plot_causal
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    slot            integer not null,
    point_class_id  integer not null
        CONSTRAINT plot_causal_element_obj_point_class_id_fk
            REFERENCES plot_point_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    object_class_id integer not null
        CONSTRAINT plot_causal_element_obj_object_class_id_fk
            REFERENCES plot_object_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id       integer
        CONSTRAINT plot_causal_element_obj_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_causal_element_obj_id_uindex
    on plot_causal_element_obj (id);

CREATE TABLE IF NOT EXISTS plot_causal_element_lit
(
    id        integer not null
        CONSTRAINT plot_causal_element_lit_pk
            PRIMARY KEY autoincrement,
    causal_id integer not null
        CONSTRAINT plot_causal_element_lit_causal_id_fk
            REFERENCES plot_causal
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    slot      integer not null,
    type      text    not null
        CHECK ( type in ('c', 'l', 'g', 'r', 'm') ),
    literal   text    not null,
    source_id integer
        CONSTRAINT plot_causal_element_lit_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_causal_element_lit_id_uindex
    on plot_causal_element_lit (id);

CREATE TABLE IF NOT EXISTS plot_causal_element_sym
(
    id        integer not null
        CONSTRAINT plot_causal_element_sym_pk
            PRIMARY KEY autoincrement,
    causal_id integer not null
        CONSTRAINT plot_causal_element_sym_causal_id_fk
            REFERENCES plot_causal
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    slot      integer not null,
    symbol_id integer not null
        CONSTRAINT plot_causal_element_sym_symbol_id_fk
            REFERENCES plot_symbol
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id integer
        CONSTRAINT plot_causal_element_sym_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_causal_element_sym_id_uindex
    on plot_causal_element_sym (id);

CREATE TABLE IF NOT EXISTS plot_causal_element_cls
(
    id              integer not null
        CONSTRAINT plot_causal_element_cls_pk
            PRIMARY KEY autoincrement,
    causal_id       integer not null
        CONSTRAINT plot_causal_element_cls_causal_id_fk
            REFERENCES plot_causal
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    slot            integer not null,
    symbol_class_id integer not null
        CONSTRAINT plot_causal_element_cls_symbol_class_id_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id       integer
        CONSTRAINT plot_causal_element_cls_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_causal_element_cls_id_uindex
    on plot_causal_element_cls (id);

CREATE TABLE IF NOT EXISTS plot_causal_element_pnt
(
    id        integer not null
        CONSTRAINT plot_causal_element_pnt_pk
            PRIMARY KEY autoincrement,
    causal_id integer not null
        CONSTRAINT plot_causal_element_pnt_causal_id_fk
            REFERENCES plot_causal
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    slot      integer not null,
    point_id  integer not null
        CONSTRAINT plot_causal_element_pnt_symbol_class_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id integer
        CONSTRAINT plot_causal_element_pnt_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_causal_element_pnt_id_uindex
    on plot_causal_element_pnt (id);

CREATE TABLE IF NOT EXISTS plot_causal_ity
(
    id        integer not null
        CONSTRAINT plot_causal_ity_pk
            PRIMARY KEY autoincrement,
    cause_id  integer not null
        CONSTRAINT plot_causal_ity_cause_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    effect_id integer not null
        CONSTRAINT plot_causal_ity_effect_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    overlap   boolean not null,
    source_id integer
        CONSTRAINT plot_causal_ity_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_causal_ity_id_uindex
    on plot_causal_ity (id);
