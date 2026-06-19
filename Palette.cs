namespace NeonGrid;

/// <summary>
/// A cyberpunk color theme. Cycle through themes at runtime with SPACE,
/// or pass one as a CLI arg: --neon | --acid | --blood | --ice.
/// </summary>
internal readonly record struct Palette(string Name, (int r, int g, int b) Head, (int r, int g, int b) Tail)
{
    private static readonly Palette[] All =
    {
        new("NEON",  Head: (235, 255, 255), Tail: (0, 255, 170)),   // classic green-cyan
        new("ACID",  Head: (255, 255, 230), Tail: (190, 255, 0)),   // toxic yellow-green
        new("BLOOD", Head: (255, 235, 235), Tail: (255, 30, 80)),   // hot magenta-red
        new("ICE",   Head: (240, 250, 255), Tail: (70, 160, 255)),  // electric blue
    };

    public static Palette From(string[] args)
    {
        var all = All;
        foreach (var a in args)
        {
            var match = all.FirstOrDefault(p => "--" + p.Name.ToLower() == a.ToLower());
            if (match.Name is not null) return match;
        }
        return all[0];
    }

    public Palette Next()
    {
        var name = Name;                       // copy 'this.Name' to a local so the
        var i = Array.FindIndex(All, p => p.Name == name);  // lambda doesn't capture 'this'
        return All[(i + 1) % All.Length];
    }

    /// <summary>Linearly interpolate from the bright head toward a dimming tail based on depth 0..1.</summary>
    public (int r, int g, int b) Shade(double depth)
    {
        depth = Math.Clamp(depth, 0, 1);
        double fade = 1.0 - depth;             // tail darkens with depth
        var h = Head;
        var t = Tail;
        int r = (int)((h.r + (t.r - h.r) * depth) * fade);
        int g = (int)((h.g + (t.g - h.g) * depth) * fade);
        int b = (int)((h.b + (t.b - h.b) * depth) * fade);
        return (r, g, b);
    }
}