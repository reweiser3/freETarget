using Microsoft.AspNetCore.Components.WebView.Maui;
using FreeETargetMaui.Services;

namespace FreeETargetMaui;

public static class MauiProgram
{
	public static MauiApp CreateMauiApp()
	{
		var builder = MauiApp.CreateBuilder();
                builder
                        .UseMauiApp<App>();

		builder.Services.AddMauiBlazorWebView();
#if DEBUG
		builder.Services.AddBlazorWebViewDeveloperTools();
#endif

        builder.Services.AddSingleton<TargetService>();

		return builder.Build();
	}
}
