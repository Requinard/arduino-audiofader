///Enum defenities
typedef enum
{
	UIT,
	AAN,
} State;
typedef enum
{
	LIN,
	LOGPO,	//120*log(x/2)
	LOGNE,	//105*log10(-x+255)
	TANH,	//tanh((x-128)/50)*128+128
} DelayType;
typedef enum
{
	FADE,
	DELAY,
	TYPE,
} Schermstate;
///Einde enumdefenities

///Structdefenities
typedef struct Settings {
	int fadeTime;
	int speakDelay;
	DelayType delayType;
};
///Einde structuredefenities

///Functiedefenities
bool inputChanged();
bool changeAudio(bool, unsigned char*, unsigned char*, Settings);
void sendWire(unsigned char*, unsigned char*);
void regelScherm(Settings*, Schermstate*);
void updateScherm(Settings*, Schermstate*);
/// einde functiedefinities