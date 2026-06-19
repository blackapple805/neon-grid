using System.Diagnostics;
using System.Text;

namespace NeonGrid;

/// <summary>
/// NEONGRID — a cyberpunk "digital rain" terminal renderer.
/// Pure C#, zero dependencies, raw ANSI escape codes, double-buffered.
/// Each column is an independent falling "stream" with a bright head and a fading neon tail.
/// </summary>
internal static class Program
{
    private static void Main(string[] args)
    {
        var palette = Palette.From(args);
        Console.OutputEncoding = Encoding.UTF8;
        Console.CursorVisible = false;
        Console.Clear();

        // Switch terminal to alternate screen buffer so the user's scrollback is preserved.
        Console.Write(Ansi.EnterAltScreen);

        var stopwatch = Stopwatch.StartNew();
        var cancel = false;

        Console.CancelKeyPress += (_, e) => { e.Cancel = true; cancel = true; };

        try
        {
            int width = 0, height = 0;
            GridRenderer? renderer = null;

            while (!cancel)
            {
                // Handle live resize and keypresses without blocking the render loop.
                if (Console.WindowWidth != width || Console.WindowHeight != height || renderer is null)
                {
                    width = Math.Max(Console.WindowWidth, 10);
                    height = Math.Max(Console.WindowHeight - 1, 5);
                    renderer = new GridRenderer(width, height, palette);
                    Console.Write(Ansi.Clear);
                }

                if (Console.KeyAvailable)
                {
                    var key = Console.ReadKey(intercept: true).Key;
                    if (key is ConsoleKey.Q or ConsoleKey.Escape) break;
                    if (key is ConsoleKey.Spacebar) palette = palette.Next();
                    renderer = new GridRenderer(width, height, palette);
                }

                renderer.Step();
                Console.Write(renderer.Compose(stopwatch.Elapsed.TotalSeconds));
                Thread.Sleep(33); // ~30 FPS
            }
        }
        finally
        {
            Console.Write(Ansi.LeaveAltScreen);
            Console.CursorVisible = true;
            Console.ResetColor();
            Console.Clear();
            Console.WriteLine("NEONGRID // session terminated. stay jacked in.\n");
        }
    }
}
