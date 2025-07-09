using System.Text.Json;

namespace FreeETargetMaui.Services;

public class Shot
{
    public int Number { get; set; }
    public decimal X { get; set; }
    public decimal Y { get; set; }
    public decimal Radius { get; set; }
    public decimal Angle { get; set; }
    public bool Miss { get; set; }

    public static bool TryParse(string json, out Shot? shot)
    {
        try
        {
            var doc = JsonDocument.Parse("{" + json + "}");
            var root = doc.RootElement;
            shot = new Shot
            {
                Number = root.GetProperty("shot").GetInt32(),
                X = root.GetProperty("x").GetDecimal(),
                Y = root.GetProperty("y").GetDecimal(),
                Radius = root.TryGetProperty("r", out var r) ? r.GetDecimal() : 0,
                Angle = root.TryGetProperty("a", out var a) ? a.GetDecimal() : 0,
                Miss = root.TryGetProperty("miss", out var m) && m.GetInt32() == 1
            };
            return true;
        }
        catch
        {
            shot = null;
            return false;
        }
    }
}
