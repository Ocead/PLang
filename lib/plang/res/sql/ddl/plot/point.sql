CREATE TABLE IF NOT EXISTS plot_point_class
(
    id        integer not null
        CONSTRAINT plot_point_class_pk
            PRIMARY KEY autoincrement,
    path_id   integer not null
        CONSTRAINT plot_point_class_path_id_fk
            REFERENCES path
            DEFERRABLE initially deferred,
    singleton boolean DEFAULT false not null,
    source_id integer
        CONSTRAINT plot_point_class_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_point_class_uindex
    on plot_point_class (id);

CREATE UNIQUE INDEX IF NOT EXISTS plot_point_class_path_id_uindex
    on plot_point_class (id, path_id);

CREATE TABLE IF NOT EXISTS plot_point_class_hint
(
    id        integer not null
        CONSTRAINT plot_point_class_hint_pk
            PRIMARY KEY autoincrement,
    class_id  integer not null
        CONSTRAINT plot_point_class_hint_class_id_fk
            REFERENCES plot_point_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    hint_id   integer not null
        CONSTRAINT plot_point_class_hint_hint_id_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    recursive boolean DEFAULT false not null,
    source_id integer
        CONSTRAINT plot_point_class_hint_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_point_class_hint_id_uindex
    on plot_point_class_hint (id);

CREATE TABLE IF NOT EXISTS plot_point
(
    id        integer not null
        CONSTRAINT plot_point_pk
            PRIMARY KEY autoincrement,
    class_id  integer not null
        CONSTRAINT plot_point_class_fk
            REFERENCES plot_point_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    truth     boolean DEFAULT true not null,
    source_id integer
        CONSTRAINT plot_point_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_point_id_uindex
    on plot_point (id);

CREATE TABLE IF NOT EXISTS plot_point_subject_sym
(
    id         integer not null
        CONSTRAINT plot_point_subject_sym_pk
            PRIMARY KEY autoincrement,
    point_id   integer not null
        CONSTRAINT plot_point_subject_sym_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    subject_id integer not null
        CONSTRAINT plot_point_subject_sym_subject_id_fk
            REFERENCES plot_symbol
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id  integer
        CONSTRAINT plot_point_subject_sym_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_point_subject_sym_id_uindex
    on plot_point_subject_sym (id);

CREATE TABLE IF NOT EXISTS plot_point_subject_cls
(
    id         integer not null
        CONSTRAINT plot_point_subject_cls_pk
            PRIMARY KEY autoincrement,
    point_id   integer not null
        CONSTRAINT plot_point_subject_cls_id_fk
            REFERENCES plot_point
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    subject_id integer not null
        CONSTRAINT plot_point_subject_cls_subject_id_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id  integer
        CONSTRAINT plot_point_subject_cls_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_point_subject_cls_id_uindex
    on plot_point_subject_cls (id);
