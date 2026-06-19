namespace NeonGrid;

/// <summary>Minimal ANSI/VT100 escape sequence helpers — true-color, cursor, screen control.</summary>
internal static class Ansi
{
    private const string Esc = "\x1b[";

    public const string Clear = Esc + "2J" + Esc + "H";
    public const string Home = Esc + "H";
    public const string EnterAltScreen = Esc + "?1049h";
    public const string LeaveAltScreen = Esc + "?1049l";
    public const string Reset = Esc + "0m";

    /// <summary>Move cursor to a 0-indexed row/col (translated to 1-indexed for the terminal).</summary>
    public static string MoveTo(int row, int col) => $"{Esc}{row + 1};{col + 1}H";

    /// <summary>24-bit foreground color.</summary>
    public static string Fg(int r, int g, int b) => $"{Esc}38;2;{r};{g};{b}m";
}
