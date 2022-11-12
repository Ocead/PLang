CREATE TABLE IF NOT EXISTS plot_object_class
(
    id             integer not null
        CONSTRAINT plot_object_class_pk
            PRIMARY KEY autoincrement,
    name           text
        CHECK ( length(name) > 0 )
        CHECK ( name regexp '^[^.:?\[\]]*$' ),
    point_class_id integer not null
        CONSTRAINT plot_object_class_class_id_fk
            REFERENCES plot_point_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    "default"      boolean not null,
    singleton      boolean DEFAULT false not null,
    ordinal        integer,
    description    text,
    source_id      integer
        CONSTRAINT plot_object_class_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_class_id_uindex
    on plot_object_class (id);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_point_class_id_path_name_uindex
    on plot_object_class (point_class_id, name);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_point_class_id_default_uindex
    on plot_object_class (point_class_id, (1)) WHERE "default" is true;

CREATE TABLE IF NOT EXISTS plot_object_class_hint_lit
(
    id        integer not null
        CONSTRAINT plot_object_class_hint_lit_pk
            PRIMARY KEY autoincrement,
    class_id  integer not null
        CONSTRAINT plot_object_class_hint_lit_class_id_fk
            REFERENCES plot_object_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    hint      text    not null,
    type      text DEFAULT 'c' not null
        CHECK ( type in ('c', 'l', 'g', 'm', 'r') ),
    source_id integer
        CONSTRAINT plot_object_class_hint_lit_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_class_hint_lit_id_uindex
    on plot_object_class_hint_lit (id);

CREATE TABLE IF NOT EXISTS plot_object_class_hint_sym
(
    id        integer not null
        CONSTRAINT plot_object_class_hint_sym_pk
            PRIMARY KEY autoincrement,
    class_id  integer not null
        CONSTRAINT plot_object_class_hint_sym_class_id_fk
            REFERENCES plot_object_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    hint_id   integer not null
        CONSTRAINT plot_object_class_hint_sym_hint_id_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    recursive boolean DEFAULT false not null,
    source_id integer
        CONSTRAINT plot_object_class_hint_sym_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_class_hint_sym_id_uindex
    on plot_object_class_hint_sym (id);

CREATE TABLE IF NOT EXISTS plot_object_class_hint_pnt
(
    id        integer not null
        CONSTRAINT plot_object_class_hint_pnt_pk
            PRIMARY KEY autoincrement,
    class_id  integer not null
        CONSTRAINT plot_object_class_hint_pnt_class_id_fk
            REFERENCES plot_object_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    hint_id   integer not null
        CONSTRAINT plot_object_class_hint_pnt_hint_id_fk
            REFERENCES plot_point_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    recursive boolean DEFAULT false not null,
    source_id integer
        CONSTRAINT plot_object_class_hint_pnt_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_class_hint_pnt_id_uindex
    on plot_object_class_hint_pnt (id);

CREATE TABLE IF NOT EXISTS plot_object_lit
(
    id        integer not null
        CONSTRAINT plot_object_lit_pk
            PRIMARY KEY autoincrement,
    point_id  integer not null
        CONSTRAINT plot_object_lit_point_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    class_id  integer not null
        CONSTRAINT plot_object_lit_class_id_fk
            REFERENCES plot_object_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    object    text,
    source_id integer
        CONSTRAINT plot_object_lit_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_lit_id_uindex
    on plot_object_lit (id);

CREATE TABLE IF NOT EXISTS plot_object_sym
(
    id        integer not null
        CONSTRAINT plot_object_sym_pk
            PRIMARY KEY autoincrement,
    point_id  integer not null
        CONSTRAINT plot_object_sym_point_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    class_id  integer not null
        CONSTRAINT plot_object_sym_class_id_fk
            REFERENCES plot_object_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    object_id integer not null
        CONSTRAINT plot_object_sym_object_id_fk
            REFERENCES plot_symbol
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id integer
        CONSTRAINT plot_object_sym_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_sym_id_uindex
    on plot_object_sym (id);

CREATE TABLE IF NOT EXISTS plot_object_cls
(
    id        integer not null
        CONSTRAINT plot_object_cls_pk
            PRIMARY KEY autoincrement,
    point_id  integer not null
        CONSTRAINT plot_object_cls_point_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    class_id  integer not null
        CONSTRAINT plot_object_cls_class_id_fk
            REFERENCES plot_object_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    object_id integer not null
        CONSTRAINT plot_object_cls_object_id_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id integer
        CONSTRAINT plot_object_cls_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_cls_id_uindex
    on plot_object_cls (id);

CREATE TABLE IF NOT EXISTS plot_object_pnt
(
    id        integer not null
        CONSTRAINT plot_object_pnt_pk
            PRIMARY KEY autoincrement,
    point_id  integer not null
        CONSTRAINT plot_object_pnt_point_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    class_id  integer not null
        CONSTRAINT plot_object_pnt_class_id_fk
            REFERENCES plot_object_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    object_id integer not null
        CONSTRAINT plot_object_pnt_object_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id integer
        CONSTRAINT plot_object_pnt_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_object_pnt_id_uindex
    on plot_object_pnt (id);
