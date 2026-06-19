using System.Text;

namespace NeonGrid;

/// <summary>
/// Owns the falling-stream simulation and turns it into a single ANSI frame string.
/// One <see cref="Stream"/> per column; each frame advances every stream and only
/// emits escape codes for cells whose glyph or color changed (diff rendering).
/// </summary>
internal sealed class GridRenderer
{
    private const string Glyphs =
        "ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉ0123456789ﾘﾙﾚﾛﾜﾝ$+=*<>";

    private readonly int _w, _h;
    private readonly Stream[] _streams;
    private readonly Random _rng = new();
    private Palette _palette;

    // Double buffer: what we drew last frame vs. what we want this frame.
    private readonly (char ch, int color)[,] _front;
    private readonly (char ch, int color)[,] _back;
    private readonly StringBuilder _sb = new(1 << 16);

    public GridRenderer(int width, int height, Palette palette)
    {
        _w = width;
        _h = height;
        _palette = palette;
        _streams = new Stream[width];
        _front = new (char, int)[height, width];
        _back = new (char, int)[height, width];

        for (int x = 0; x < width; x++)
            _streams[x] = Stream.Spawn(_rng, height);
    }

    public void Step()
    {
        // Clear the back buffer.
        for (int y = 0; y < _h; y++)
            for (int x = 0; x < _w; x++)
                _back[y, x] = ('\0', 0);

        for (int x = 0; x < _w; x++)
        {
            ref var s = ref _streams[x];
            s.Advance(_rng);
            if (s.Head - s.Length > _h) s = Stream.Spawn(_rng, _h, respawn: true);

            for (int t = 0; t < s.Length; t++)
            {
                int y = s.Head - t;
                if (y < 0 || y >= _h) continue;

                double depth = (double)t / s.Length;
                var (r, g, b) = _palette.Shade(depth);
                int packed = (r << 16) | (g << 8) | b;
                char ch = (t == 0) ? '\u2588' : Glyphs[_rng.Next(Glyphs.Length)];
                if (t == 0) packed = 0xFFFFFF; // bright white head
                _back[y, x] = (ch, packed);
            }
        }
    }

    /// <summary>Diff back buffer against front buffer and emit a minimal ANSI frame.</summary>
    public string Compose(double seconds)
    {
        _sb.Clear();
        int lastColor = -1;

        for (int y = 0; y < _h; y++)
        {
            for (int x = 0; x < _w; x++)
            {
                var cell = _back[y, x];
                if (cell == _front[y, x]) continue;

                _sb.Append(Ansi.MoveTo(y, x));
                if (cell.ch == '\0')
                {
                    _sb.Append(' ');
                }
                else
                {
                    if (cell.color != lastColor)
                    {
                        _sb.Append(Ansi.Fg((cell.color >> 16) & 0xFF, (cell.color >> 8) & 0xFF, cell.color & 0xFF));
                        lastColor = cell.color;
                    }
                    _sb.Append(cell.ch);
                }
                _front[y, x] = cell;
            }
        }

        // HUD line at the bottom.
        var (hr, hg, hb) = _palette.Tail;
        _sb.Append(Ansi.MoveTo(_h, 0)).Append(Ansi.Fg(hr, hg, hb));
        _sb.Append($"NEONGRID //{_palette.Name,-6}// {_w}x{_h} // {seconds,6:0.0}s // [SPACE] theme  [Q] quit   ");
        _sb.Append(Ansi.Reset);
        return _sb.ToString();
    }
}

/// <summary>A single falling column of glyphs with a moving head and a trailing length.</summary>
internal struct Stream
{
    public int Head;       // current bottom-most lit row
    public int Length;     // tail length in rows
    public int Speed;      // rows advanced every N frames
    private int _tick;

    public static Stream Spawn(Random rng, int height, bool respawn = false) => new()
    {
        Head = respawn ? rng.Next(-height, 0) : rng.Next(-height, height),
        Length = rng.Next(height / 4, Math.Max(height / 4 + 1, height)),
        Speed = rng.Next(1, 4),
        _tick = 0,
    };

    public void Advance(Random rng)
    {
        if (++_tick % Speed != 0) return;
        Head++;
    }
}
