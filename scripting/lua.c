#include <string.h>
#include "../nessemble.h"

#ifndef IS_JAVASCRIPT
#ifndef IS_WINDOWS
#include <lua.h>
#else /* IS_WINDOWS */
#include "lua.h"
#endif
#include <lauxlib.h>
#include <lualib.h>
#endif /* IS_JAVASCRIPT */

unsigned int scripting_lua(char *exec) {
    unsigned int rc = RETURN_OK;

#ifndef IS_JAVASCRIPT
    int error = 0;
    unsigned int i = 0, l = 0;
    size_t return_len = 0;
    char *return_str = NULL, *error_str = NULL;

    lua_State *L;

    L = luaL_newstate();
    luaL_openlibs(L);

    if ((error = luaL_loadfile(L, exec)) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((error = lua_pcall(L, 0, 0, 0)) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 0, l = length_texts; i < l; i++) {
        lua_getglobal(L, "add_string");

        if (!lua_isnil(L, -1)) {
            lua_pushstring(L, texts[i]);

            if ((error = lua_pcall(L, 1, 0, 0) != 0)) {
                rc = RETURN_EPERM;
                goto cleanup;
            }
        }
    }

    lua_getglobal(L, "custom");

    for (i = 0, l = length_ints; i < l; i++) {
        lua_pushnumber(L, ints[i]);
    }

    if ((error = lua_pcall(L, length_ints, 1, 0)) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    return_str = (char *)lua_tolstring(L, -1, &return_len);

    for (i = 0, l = (unsigned int)return_len; i < l; i++) {
        write_byte((unsigned int)return_str[i] & 0xFF);
    }

cleanup:
    if (error != 0) {
        error_str = strchr(lua_tostring(L, -1), ':');
        error_str = strchr(error_str+2, ':');

        yyerror(error_str+2);
    }

    lua_close(L);

#endif /* IS_JAVASCRIPT */
    return rc;
}