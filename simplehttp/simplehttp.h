#ifndef SIMPLEHTTP_H_
#define SIMPLEHTTP_H_

class ScriptablePluginObject;

bool SayHello(ScriptablePluginObject* obj, const NPVariant* args,
				unsigned int argCount, NPVariant* result);

bool HttpRequest(ScriptablePluginObject* obj, const NPVariant* args,
				unsigned int argCount, NPVariant* result);

#endif //SIMPLEHTTP_H