from sqlalchemy import event
from sqlalchemy.engine import Engine

try:
    from plang.db.base import *
    from plang.db.models import *
    from plang.db.plot import *
    from plang.db.story import *
    from plang.db.outline import *
    from plang.db.text import *
except ImportError as e:
    print(e)


@event.listens_for(Engine, "connect")
def __set_sqlite_pragma(dbapi_connection, *_):
    cursor = dbapi_connection.cursor()
    cursor.execute("PRAGMA defer_foreign_keys=ON")
    cursor.execute("PRAGMA foreign_keys=ON")
    cursor.execute("PRAGMA recursive_triggers=ON")
    cursor.close()
