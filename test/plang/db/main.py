from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

from plang.db import *


def main():
    engine = create_engine(f'sqlite:///test.sqlite3')
    Base.metadata.create_all(engine)

    Session = sessionmaker()
    Session.configure(bind=engine)
    session = Session()

    session.commit()


if __name__ == '__main__':
    main()
