/*
 * Network code module
 */

#include <stdlib.h>
#include <string.h>

#include <map>

#include "common.h"
#include "netalg.h"
#include "netcode.h"
#include "game.h"

#define CONNECTED (tokenring && tokenring->connected())

namespace NetCode {

using namespace std;

TokenRing *tokenring = NULL;
uword port = GAME_PORT;

//------------------------------------------------------------------------------

struct Receive
{
	typedef void(*Func) (NodeID, Message &, bool);
	typedef map<string,Func> List;
	
	static List list;
	
	Receive(string cmd, Func func) { list[cmd] = func; }
};

Receive::List Receive::list;

#define RECEIVE(cmd, id, m, r)                   \
	void _ ## cmd(NodeID, Message &, bool);      \
	Receive __ ## cmd(#cmd, _ ## cmd);           \
	void _ ## cmd(NodeID id, Message &m, bool r)

#define SEND(msg,rel) {if (tokenring) tokenring->shout(msg,rel);}

//==============================================================================

void Initialize(int argc, char *argv[])
{
	string host;
	
	for (int i = 0; i < argc - 1; ++i)
	{
		if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--connect"))
			host = argv[++i];
		if (!strcmp(argv[i], "-P") || !strcmp(argv[i], "--port"))
			port = atoi(argv[++i]);
	}
	
	tokenring = new TokenRing(port);
	
	if (host.empty())   // Server
		Game::Notice(string("Listening on port ") + Argument(port).str + string("..."));
	else Connect(host); //Client
}

//------------------------------------------------------------------------------

void Terminate()
{
	if (tokenring) delete tokenring;
	tokenring = NULL;
}

//------------------------------------------------------------------------------

void Frame()
{
	if (!CONNECTED) return;
	
	NodeID id;
	Message msg;
	bool reliable;
	
	while (tokenring->recvfrom(id, msg, reliable))
		if (Receive::list.count(msg[0]))
			Receive::list[msg[0]](id, msg, reliable);
	
	while (tokenring->loss(id))
	{
	}
	
	while (tokenring->entry(id))
	{
		// Temporarily add a player to the world
		game.world->children.insert(Player(id));
	}
}

//------------------------------------------------------------------------------

bool TryLock()
{
	if (!tokenring) return false;
	return tokenring->authorized();
}

//------------------------------------------------------------------------------

void Unlock()
{
	if (tokenring) tokenring->pass();
}

//==============================================================================

bool Connected()
{
	return CONNECTED;
}

//------------------------------------------------------------------------------

void Disconnect()
{
	if (tokenring) delete tokenring;
	tokenring = new TokenRing(port);
	Game::Notice(string("Disconnected: listening on port ") + Argument(port).str + string("..."));
}

//------------------------------------------------------------------------------

bool Connect(std::string host)
{
	if (CONNECTED) Disconnect();
	Game::Notice(string("Connecting to ") + host + string("..."));
	Address remote(host.c_str());
	if (!tokenring->connect(remote), 30)
	{
		Game::Notice(string("Unable to connect to " + host + string("!")));
		return false;
	}
	return true;
}

//==============================================================================

bool Send(const Message &msg, bool reliable)
{
	if (!CONNECTED) return false;
	return tokenring->shout(msg, reliable);
}

//------------------------------------------------------------------------------

void Chat(string line)
{
	Message msg;
	msg.push_back("CHAT");
	msg.push_back(line);
	SEND(msg, false);
	DisplayChatMsg(game.player, msg[1]);
}
RECEIVE(CHAT, id, msg, reliable)
{
	DisplayChatMsg(Player::byId((Player::Id) id), msg[1]);
}

//------------------------------------------------------------------------------

void Move(Pd position, Vd velocity)
{
	if (!game.player) return;
	
	Message msg;
	msg.push_back("MOVE");
	msg.push_back(game.player->origin.x);
	msg.push_back(game.player->origin.Y);
	msg.push_back(game.player->origin.Z);
	msg.push_back(0);
	msg.push_back(0);
	msg.push_back(0);
	SEND(msg, false);
}
RECEIVE(MOVE, id, msg, reliable)
{
	Pd position((double) msg[1], (double) msg[2], (double) msg[3]);
	Vd velocity((double) msg[4], (double) msg[5], (double) msg[6]);
	
	Player *player = Player::byId(id);
	if (!player) return;
	
	player->origin = position;
}

//------------------------------------------------------------------------------

} // namespace NetCode

//------------------------------------------------------------------------------
