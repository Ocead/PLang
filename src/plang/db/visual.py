class OP:
    @classmethod
    def path(cls, rich: bool = False) -> str:
        return r'[bold][blue].[/blue][/bold]' if rich else '.'

    @classmethod
    def recur(cls, rich: bool = False) -> str:
        return r'[bold][blue]...[/blue][/bold]' if rich else '...'

    @classmethod
    def list(cls, rich: bool = False) -> str:
        return r'[gray].[/gray]' if rich else ','

    @classmethod
    def obj(cls, rich: bool = False) -> str:
        return r'[bold].[/bold]' if rich else ':'

    @classmethod
    def decl(cls, rich: bool = False) -> str:
        return r'[bold].[/bold]' if rich else ';'

    @classmethod
    def obj_name(cls, rich: bool = False) -> str:
        return r'[bold][blue].[/blue][/bold]' if rich else '?'

    @classmethod
    def single(cls, rich: bool = False) -> str:
        return r'[bold][red].[/red][/bold]' if rich else '!'

    @classmethod
    def negate(cls, rich: bool = False) -> str:
        return r'[bold][red].[/red][/bold]' if rich else '~'

    @classmethod
    def hint_l(cls, rich: bool = False) -> str:
        return r'[italic][bold][gray]([/gray][/bold]' if rich else '('

    @classmethod
    def hint_r(cls, rich: bool = False) -> str:
        return r'[bold][gray])[/gray][/bold][/italic]' if rich else ')'

    @classmethod
    def sym_l(cls, rich: bool = False) -> str:
        return r'[bold][blue]\[[/blue][/bold]' if rich else '['

    @classmethod
    def sym_r(cls, rich: bool = False) -> str:
        return r'[bold][blue]][/blue][/bold]' if rich else ']'

    @classmethod
    def sym(cls, rich: bool = False) -> str:
        return r'[bold][blue]\[][/blue][/bold]' if rich else '[]'

    @classmethod
    def pnt_l(cls, rich: bool = False) -> str:
        return r'[bold][blue]{[/blue][/bold]' if rich else '{'

    @classmethod
    def pnt_r(cls, rich: bool = False) -> str:
        return r'[blue][bold]}[/blue][/bold]' if rich else '}'

    @classmethod
    def pnt(cls, rich: bool = False) -> str:
        return r'[bold][blue]{}[/blue][/bold]' if rich else '{}'

    @classmethod
    def id(cls, rich: bool = False) -> str:
        return r'[bold][blue]*[/blue][/bold]' if rich else '*'
