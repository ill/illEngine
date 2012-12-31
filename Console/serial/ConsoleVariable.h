#ifndef ILL_VARIABLE_H__
#define ILL_VARIABLE_H__

#include <string>

namespace Console {

/**
Base type for a console variable value.

Normally this would be a templated class with m_value being the template type
but then you can't have the ConsoleVariable class have a reference to ConsoleVariableValue
without also specifying the type.

So the way to do it is to inherit from ConsoleVariableValue and use static_cast<>
to get the correct class for the stored value.
*/
struct ConsoleVariableValue {
   //TODO: add references to callbacks for autocomplete and onChange and all that...
};

struct ConsoleVariableBoolean : public ConsoleVariableValue {
   ConsoleVariableBoolean(bool value = false)
      : ConsoleVariableValue(),
      m_value(value)
   {}

   bool m_value;
};

struct ConsoleVariableString : public ConsoleVariableValue {
   ConsoleVariableString(const char * value = "")
      : ConsoleVariableValue(),
      m_value(value)
   {}

   std::string m_value;
};

struct ConsoleVariableInt : public ConsoleVariableValue {
   ConsoleVariableInt(int value = 0)
      : ConsoleVariableValue(),
      m_value(value)
   {}

   int m_value;
};

struct ConsoleVariableFloat : public ConsoleVariableValue {
   ConsoleVariableFloat(float value = 0.0f)
      : ConsoleVariableValue(),
      m_value(value)
   {}

   float m_value;
};

/**
The console variable itself.
Manages the ConsoleVariableValue.
The value object must be created outside, but this class itself frees it on destruction.
*/
struct ConsoleVariable {
public:
   inline ConsoleVariable(ConsoleVariableValue* value = NULL, const char * description = "")
      : m_value(value),
      m_description(description)
   {}

   inline virtual ~ConsoleVariable() {
      delete m_value;
   }

   //TODO: make thread safe
   inline const ConsoleVariableValue * getValue() const {
      return m_value;
   }

   inline const char * getDescription() const {
      return m_description.c_str();
   }

private:
   ///value
   ConsoleVariableValue * m_value;

   ///variable description that shows up when someone views it in the dev console
   std::string m_description;
};

}

#endif