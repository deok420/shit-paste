#pragma once
const char* AimType[] =
{
	"Hitbox",
	"Nearest hitbox"
};

const char* LegitHitbox[] =
{
	"Head",
	"Neck",
	"Pelvis",
	"Stomach",
	"Lower chest",
	"Chest",
	"Upper chest"
};

const char* dtmode[] =
{
	"Defensive",
	"Offensive",
	"Aggresive"
};

const char* hitboxes[] =
{
	"Head",
	"Upper chest",
	"Chest",
	"Lower chest",
	"Stomach",
	"Pelvis",
	"Arms",
	"Legs",
	"Feet"
};

const char* autoscope_type[] =
{
	"Regular",
	"Dynamic",
};
const char* bodyaimlevel[] =
{
	"Low",
	"Medium",
	"High"
};

const char* bodyaim[] =
{
	"In air",
	"On high velocity",
	"On lethal",
	"On double tap",
	"If unresolved",
	"Prefer"
};

const char* safe_points_conditions[] =
{
	"On lethal",
	"Visible",
	"In air",
	"In crouch",
	"After x misses",
	"If hp < x",
	"On limbs"
};

const char* bodyaimcond[] =
{
	"Double tap",
	"Prefer",
	"lethal"
};

const char* headaimcond[] =
{
	"On shot",
	"Running"
};

const char* headaimonlycond[] =
{
	"On shot",
	"Running"
};

const char* LegitSelection[] =
{
	"Near crosshair",
	"Lowest health",
	"Highest damage",
	"Lowest distance"
};

const char* antiaim_type[] =
{
	"Rage",
	"Legit"
};

const char* movement_type[] =
{
	"Stand",
	"Slow walk",
	"Move",
	"Air"
};

const char* LegitFov[] =
{
	"Static",
	"Dynamic"
};

const char* LegitSmooth[] =
{
	"Static",
	"Humanized"
};

const char* RCSType[] =
{
	"Always on",
	"On target"
};

const char* selection[] =
{
	"Cycle",
	"Near crosshair",
	"Lowest distance",
	"Lowest health",
	"Highest damage"
};

const char* autostop_modifiers[] =
{
	"Between shots",
	"On lethal",
	"Visible",
	"Center",
	"Force accuracy",
	"Predictive"
};

const char* multichlen[] =
{
	"Head",
	"Chest",
	"Stomach",
	"Legs",
	"Feet"
};


const char* pitch[] =
{
"None",
"Down",
"Up",
};

const char* desyncdir[] =
{
"Manual",
"Freestand",
};

const char* baseangle[] =
{
"Local view",
"Freestand",
"At targets"
};

const char* desync[] =
{
"None",
"Static",
"Jitter"
};

const char* yaw[] =
{
	"Static",
	"Jitter",
	"Spin"
};

const char* lby_type[] =
{
"Normal",
"Opposite",
"Sway"
};

const char* proj_combo[] =
{
	"Icon",
	"Text",
	"Box",
	"Glow"
};

const char* weaponplayer[] =
{
	"Icon",
	"Distance",
	"Text"
};

const char* weaponesp[] =
{
	"Icon",
	"Text",
	"Box",
	"Distance",
	"Glow",
	"Ammo"
};

const char* hitmarkers[] =
{
	"Crosshair",
	"World"
};

const char* glowtarget[] =
{
	"Enemy",
	"Teammate",
	"Local"
};

const char* glowtype[] =
{
	"Standard",
	"Pulse",
	"Inner"
};

const char* local_chams_type[] =
{
	"Real",
	"Desync"
};

const char* chamsvisact[] =
{
	"Visible",
	"Invisible"
};

const char* chamsvis[] =
{
	"Visible"
};

const char* chamstype[] =
{
	"Regular",
	"Mettalic",
	"Flat",
	"Pulse",
	"Crystal",
	"Glass",
	"Circuit",
	"Golden",
	"Glow"
};

const char* flags[] =
{
	"Money",
	"Armor",
	"Defuse kit",
	"Scoped",
	"Fakeducking",
	"Vulnerable",
	"Delay",
	"Bomb carrier"
};

const char* removals[] =
{
	"Scope",
	"Zoom",
	"Smoke",
	"Flash",
	"Recoil",
	"Landing bob",
	"Postprocessing",
	"Fog"
};

const char* indicators[] =
{
	"Fake",
	"Damage override",
	"Safe points",
	"Body aim",
	"Double tap",
	"Hide shots",
	"Duck",
	"freestand"
};

const char* skybox[] =
{
	"None",
	"Tibet",
	"Baggage",
	"Italy",
	"Aztec",
	"Vertigo",
	"Daylight",
	"Daylight 2",
	"Clouds",
	"Clouds 2",
	"Gray",
	"Clear",
	"Canals",
	"Cobblestone",
	"Assault",
	"Clouds dark",
	"Night",
	"Night 2",
	"Night flat",
	"Dusty",
	"Rainy",
	"Custom"
};

const char* mainwep[] =
{
	"None",
	"Auto",
	"AWP",
	"SSG 08"
};

const char* slowwalk_type[] =
{
	"Accuracy",
	"Custom"
};

const char* secwep[] =
{
	"None",
	"Dual Berettas",
	"Deagle/Revolver"
};

const char* strafes[] =
{
	"None",
	"Legit",
	"Rage"
};

const char* events_output[] =
{
	"Console",
	"Chat"
};

const char* events[] =
{
	"Player hits",
	"Items",
	"Bomb"
};

const char* grenades[] =
{
	"Grenades",
	"Armor",
	"Taser",
	"Defuser"
};

const char* grenades1[] =
{
	"None",
	"Beam",
	"Line"
};

const char* fakelags[] =
{
	"Static",
	"Random",
	"Dynamic",
	"Fluctuate"
};

const char* lagstrigger[] =
{
	"Slow walk",
	"Move",
	"Air",
	"Peek"
};

const char* sounds[] =
{
	"None",
	"Mettalic",
	"Cod",
	"Bubble",
	"Stapler",
	"Bell",
	"Flick"
};

const char* player_model_t[] =
{
"None",
"Enforcer",
"Soldier",
"Ground Rebel",
"Maximus",
"Osiris",
"Slingshot",
"Dragomir",
"Blackwolf",
"Prof. Shahmat",
"Rezan The Ready",
"Doctor Romanov",
"Mr. Muhlik",
"Darryl Miami",
"Number K",
"Safecracker Voltzman"

};


const char* player_model_ct[] =
{
"None",
"Seal Team 6",
"3rd Commando",
"Operator FBI",
"Squadron Officer",
"Markus Delrow",
"Buckshot",
"McCoy",
"Commander Ricksaw",
"Agent Ava",
"Biosecurity Specialist",
"Chemical protection specialist",
"Blueberry Buckshot",
"Sergeant Bombson",
"John Kus",
"Senior Lieutenant Farlow",
"Commander May Jamison",
};