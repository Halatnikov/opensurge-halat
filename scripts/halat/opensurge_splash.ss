// -----------------------------------------------------------------------------
// File: opensurge_splash.ss
// Description: 
// Author: 
// License: MIT
// -----------------------------------------------------------------------------

using SurgeEngine;
using SurgeEngine.UI.Text;
using SurgeEngine.Transform;
using SurgeEngine.Vector2;
using SurgeEngine.Video.Screen;

object "Open Surge Splash" is "setup"
{
	powered_by = spawn("Powered by");
	website = spawn("Website");
	versions = spawn("Versions");
	
	splash = spawn("Splash");
}

object "Powered by" is "entity" , "private", "detached"
{
	transform = Transform();
    text = Text("GoodNeighbors");

    fun constructor()
    {
        text.text = "Powered by";
        text.align = "center";
		transform.position = Vector2(Screen.width/2, 50);
    }
}

object "Website" is "entity" , "private", "detached"
{
	transform = Transform();
    text = Text("BoxyBold");

    fun constructor()
    {
        text.text = "$ENGINE_WEBSITE";
        text.align = "center";
		transform.position = Vector2(Screen.width/2, 114);
    }
}

object "Versions" is "entity" , "private", "detached"
{
	transform = Transform();
    text = Text("GoodNeighbors");

    fun constructor()
    {
        text.text = "Open Surge Engine " + SurgeEngine.version + 
					"\nSurgeScript " + SurgeScript.version + 
					"\nAllegro $ALLEGRO_VERSION";
        text.align = "center";
		transform.position = Vector2(Screen.width/2, 136);
    }
}