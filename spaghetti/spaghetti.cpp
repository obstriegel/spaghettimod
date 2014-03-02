#include "cube.h"
#include "spaghetti.h"
#include "commandhijack.h"
#include <csignal>

extern ENetHost* serverhost;

namespace spaghetti{

using namespace luabridge;

lua_State* L;
bool quit = false;
hashtable<const char*, ident_bind*>* idents;

ident_bind::ident_bind(const char* name){
    if(!idents) idents = new hashtable<const char*, ident_bind*>;
    (*idents)[name] = this;
}

void init(){

    auto quitter = [](int){ quit = true; };
    auto noop = [](int){};
    signal(SIGTERM, quitter);
    signal(SIGHUP,  noop);
    signal(SIGINT,  quitter);
    signal(SIGUSR1, noop);
    signal(SIGUSR2, noop);

    L = luaL_newstate();
    if(!L) fatal("Cannot create lua state");
    luaL_openlibs(L);

    try{
        auto cs = getGlobalNamespace(L).beginNamespace("cs");
        enumeratekt((*idents), const char*, name, ident_bind*, id, id->bind(name, cs));
        cs.endNamespace();
    }
    catch(const std::exception& e){ fatal("Error while binding to lua: %s", e.what()); }

    try{ getGlobal(L, "dofile")("script/bootstrap.lua"); }
    catch(const std::exception& e){
        conoutf(CON_ERROR, "Error invoking bootstrap.lua: %s\nIt's unlikely that the server will function properly.", e.what());
    }

}

void fini(){
    kicknonlocalclients();
    enet_host_flush(serverhost);
    lua_close(L);
    L = 0;
    DELETEP(idents);
}

}

using namespace spaghetti;

void setvar(const char* name, int i, bool dofunc, bool doclamp){
    (*idents)[name]->erased_setter(&i, dofunc, doclamp);
}

void setfvar(const char* name, float i, bool dofunc, bool doclamp){
    (*idents)[name]->erased_setter(&i, dofunc, doclamp);
}

void setsvar(const char* name, const char* val, bool dofunc){
    (*idents)[name]->erased_setter(val, dofunc);
}
