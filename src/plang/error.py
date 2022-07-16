class PlangException(Exception):
    def __init__(self, key: str, num: int):
        self.key = key
        self.num = num


class CLIExceptions:
    key = 'CLI'

    class UnknownCommandException(PlangException):
        def __init__(self):
            super().__init__(CLIExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: Unknown command."

    class MalformedCommandException(PlangException):
        def __init__(self):
            super().__init__(CLIExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: Malformed command."

    class NoSessionException(PlangException):
        def __init__(self):
            super().__init__(CLIExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: No active session! Open a file first."

    class ReadFileException(PlangException):
        def __init__(self):
            super().__init__(CLIExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: Could not read from file."

    class WriteFileException(PlangException):
        def __init__(self):
            super().__init__(CLIExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: Could not write to file."


class LangExceptions:
    key = 'LNG'

    class MalformedExpressionException(PlangException):
        def __init__(self):
            super().__init__(LangExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: Expression is malformed."

    class InvalidReferenceException(PlangException):
        def __init__(self):
            super().__init__(LangExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: Could not find a matching entry."

    class AmbiguousReferenceException(PlangException):
        def __init__(self):
            super().__init__(LangExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: Reference is ambiguous."

    class AlreadyDefinedException(PlangException):
        def __init__(self):
            super().__init__(LangExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: Entry is already defined."

    class HintMatchException(PlangException):
        def __init__(self):
            super().__init__(LangExceptions.key, None)

        def __str__(self) -> str:
            return "ERROR: Entry does not match any hint."
