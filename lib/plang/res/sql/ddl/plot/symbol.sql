CREATE TABLE IF NOT EXISTS plot_symbol_class
(
    id        integer not null
        CONSTRAINT plot_symbol_class_pk
            PRIMARY KEY autoincrement,
    path_id   integer not null
        CONSTRAINT plot_symbol_class_path_id_fk
            REFERENCES path
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    source_id integer
        CONSTRAINT plot_symbol_class_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_symbol_class_uindex
    on plot_symbol_class (id);

CREATE UNIQUE INDEX IF NOT EXISTS plot_symbol_class_path_id_uindex
    on plot_symbol_class (path_id);

CREATE TABLE IF NOT EXISTS plot_symbol_class_hint
(
    id          integer not null
        CONSTRAINT plot_symbol_class_hint_pk
            PRIMARY KEY autoincrement,
    class_id    integer not null
        CONSTRAINT plot_symbol_class_hint_class_id_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    hint_id     integer not null
        CONSTRAINT plot_symbol_class_hint_class_id_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    "recursive" boolean not null,
    source_id   integer
        CONSTRAINT plot_symbol_class_hint_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_symbol_class_hint_uindex
    on plot_symbol_class_hint (id);

CREATE TABLE IF NOT EXISTS plot_symbol
(
    id        integer not null
        CONSTRAINT plot_symbol_pk
            PRIMARY KEY autoincrement,
    name      text    not null
        CHECK (name regexp '^[^\[\]]*$'),
    class_id  integer not null
        CONSTRAINT plot_symbol_class_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    ordinal   integer,
    source_id integer
        CONSTRAINT plot_symbol_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_symbol_id_uindex
    on plot_symbol (id);

CREATE UNIQUE INDEX IF NOT EXISTS plot_symbol_class_id_name_uindex
    on plot_symbol (class_id, name);

CREATE TABLE IF NOT EXISTS plot_symbol_compound
(
    id          integer not null
        CONSTRAINT plot_symbol_compound_pk
            PRIMARY KEY autoincrement,
    symbol_id   integer not null
        CONSTRAINT plot_symbol_compound_class_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    compound_id integer not null
        CONSTRAINT plot_symbol_compound_class_fk
            REFERENCES plot_symbol_class
            ON DELETE CASCADE
            DEFERRABLE initially deferred,
    distance    integer not null
        CHECK ( distance >= 0 ),
    source_id   integer
        CONSTRAINT plot_symbol_compound_source_id_fk
            REFERENCES source
            ON DELETE CASCADE
            DEFERRABLE initially deferred
);

CREATE UNIQUE INDEX IF NOT EXISTS plot_symbol_compound_id_uindex
    on plot_symbol_compound (id);

CREATE UNIQUE INDEX IF NOT EXISTS plot_symbol_symbol_id_compound_id_distance_uindex
    on plot_symbol_compound (symbol_id, compound_id, distance);
