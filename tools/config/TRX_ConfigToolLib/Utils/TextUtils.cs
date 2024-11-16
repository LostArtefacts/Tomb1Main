using System.Globalization;
using System.Text;

namespace TRX_ConfigToolLib.Utils;

public static class TextUtilities
{
    public static string Normalise(string s)
    {
        return new string(s.Normalize(NormalizationForm.FormD)
            .Where(c => CharUnicodeInfo.GetUnicodeCategory(c) != UnicodeCategory.NonSpacingMark)
            .ToArray());
    }
}
