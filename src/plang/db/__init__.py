from sqlalchemy import event
from sqlalchemy.engine import Engine




@event.listens_for(Engine, "connect")
def __set_sqlite_pragma(dbapi_connection, *_):
    cursor = dbapi_connection.cursor()
    cursor.execute("PRAGMA defer_foreign_keys=ON")
    cursor.execute("PRAGMA foreign_keys=ON")
    cursor.execute("PRAGMA recursive_triggers=ON")
    cursor.close()
