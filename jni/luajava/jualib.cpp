#include "jua.h"
#include "juaapi.h"
#include "jualib.h"

#include <cstring>
#include <string>

static int javaMethod(lua_State * L) {
  if (luaL_testudata(L, 1, JAVA_OBJECT_META_REGISTRY) != NULL) {
    return jobjectSigCall(L);
  }

  if (luaL_testudata(L, 1, JAVA_CLASS_META_REGISTRY) != NULL) {
    return jclassSigCall(L);
  }

  return luaL_error(L, "bad argument #1 to 'java.method': %s or %s expected",
    JAVA_CLASS_META_REGISTRY, JAVA_OBJECT_META_REGISTRY);
}

static int javaNew(lua_State * L) {
  if (luaL_testudata(L, 1, JAVA_CLASS_META_REGISTRY) != NULL
    || luaL_testudata(L, 1, JAVA_OBJECT_META_REGISTRY) != NULL) {
    return checkOrError(L, jclassCall(L));
  } else {
    return luaL_error(L, "bad argument #1 to 'java.new': %s or %s expected",
      JAVA_CLASS_META_REGISTRY, JAVA_OBJECT_META_REGISTRY);
  }
}

static int javaLuaify(lua_State * L) {
  JNIEnv * env = getJNIEnv(L);
  int stateIndex = getStateIndex(L);
  return checkOrError(L, env->CallStaticIntMethod(juaapi_class, juaapi_luaify, (jint) stateIndex));
}

/**
 * @brief Counts the number of trailing ".*"
 */
static unsigned int countDepth(const char * str, std::size_t length) {
  int depth = 0;
  for (int i = length - 2; i >= 0; i -= 2) {
    if (str[i] == '.' && str[i + 1] == '*') {
      depth++;
    } else {
      return depth;
    }
  }
  return depth;
}

int javaImport(lua_State * L) {
  const char * className = luaL_checkstring(L, 1);

  std::size_t length = (int) std::strlen(className);
  std::size_t depth = countDepth(className, length);

  if (depth > 0) {
    /* Pre-allocates two extra slots */
    lua_createtable(L, 0, 4);
    lua_pushinteger(L, depth);
    lua_rawseti(L, -2, 1);
    std::string packageName{className, length - 2 * depth + 1};
    lua_pushstring(L, packageName.c_str());
    lua_rawseti(L, -2, 2);
    luaL_getmetatable(L, JAVA_PACKAGE_META_REGISTRY);
    lua_setmetatable(L, -2);
    return 1;
  }

  JNIEnv * env = getJNIEnv(L);
  int stateIndex = getStateIndex(L);

  jstring str = env->NewStringUTF(className);
  int ret = env->CallStaticIntMethod(juaapi_class, juaapi_import, (jint) stateIndex,
                                     str);
  env->DeleteLocalRef(str);
  return checkOrError(L, ret);
}

static int javaProxy(lua_State * L) {
  JNIEnv * env = getJNIEnv(L);
  int stateIndex = getStateIndex(L);
  return checkOrError(L, env->CallStaticIntMethod(juaapi_class, juaapi_proxy, (jint) stateIndex));
}

static int javaArray(lua_State * L) {
  if (luaL_testudata(L, 1, JAVA_CLASS_META_REGISTRY) != NULL
    || luaL_testudata(L, 1, JAVA_OBJECT_META_REGISTRY) != NULL) {
    JNIEnv * env = getJNIEnv(L);
    int stateIndex = getStateIndex(L);
    int top = lua_gettop(L);
    jobject * data = (jobject *) lua_touserdata(L, 1);
    if (top == 2) {
      return checkOrError(L, env->CallStaticIntMethod(juaapi_class, juaapi_arraynew,
        (jint) stateIndex, *data, (jint) lua_tointeger(L, 2)));
    }
    if (top > 2) {
      return checkOrError(L, env->CallStaticIntMethod(juaapi_class, juaapi_arraynew,
        (jint) stateIndex, *data, (jint) (1 - top)));
    }
    return luaL_error(L, "bad argument #2 to 'java.array': number expected, got none");
  }
  return luaL_error(L, "bad argument #1 to 'java.array': %s or %s expected",
    JAVA_CLASS_META_REGISTRY, JAVA_OBJECT_META_REGISTRY);
}

const luaL_Reg javalib[] = {
  { "method",    javaMethod },
  { "new",       javaNew },
  { "luaify",    javaLuaify },
  { "import",    javaImport },
  { "proxy",     javaProxy },
  { "array",     javaArray },
  {NULL, NULL}
};
