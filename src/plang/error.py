class PlangException(Exception):
    pass


class NoSessionException(PlangException):
    def __str__(self) -> str:
        return "ERROR: No active session! Open a file first."

