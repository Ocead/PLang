try:
    from plang.lang.generated.PlangLexer import *
    from plang.lang.generated.PlangParser import *
    from plang.lang.generated.PlangListener import *
    from plang.lang.generated.PlangVisitor import *
except ImportError as e:
    raise ImportError('Could not import the generated parser.\n'
                      'Generate the ANTLR4 recognizer for lang/Plang.g4 here!') from e
