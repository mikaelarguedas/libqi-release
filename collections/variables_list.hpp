/*
 * variables_list.hpp
 *
 *  Created on: Oct 19, 2009 at 11:31:46 AM
 *      Author: Jean-Charles DELAY
 * 			Mail  : jdelay@aldebaran-robotics.com
 */

#ifndef LIBIPPC_VARIABLESLIST_HPP_
#define LIBIPPC_VARIABLESLIST_HPP_

#include <alcommon-ng/collections/equality_visitor.hpp>
#include <alcommon-ng/exceptions/exceptions.hpp>

#include <list>
#include <vector>
#include <map>
#include <string>
#include <typeinfo>

#ifndef foreach
# include <boost/foreach.hpp>
# define foreach  BOOST_FOREACH
#endif

namespace AL {
  namespace Messaging {

    class VariableValue;

    struct EmptyValue {
      bool operator==(const EmptyValue& rhs) const {
        (void) rhs;
        return true;
      }
    };

    typedef boost::variant<bool,
                           int,
                           float,
                           double,
                           std::string,
                           std::vector<unsigned char>,
                           std::vector<VariableValue>,
                           std::map<std::string, std::string>,
                           EmptyValue
                           > ValueType;

    /** The generic serializable type to use to transmit parameters. */
    class VariableValue {
    public:
      VariableValue()         { _value = EmptyValue(); }
      VariableValue(bool b)   { _value = b; }
      VariableValue(int i)    { _value = i; }
      VariableValue(float f)  { _value = f; }
      VariableValue(double d) { _value = d; }
      VariableValue(const std::string & s)                        { _value = s; }
      VariableValue(const std::vector<unsigned char> & bin)       { _value = bin; }
      VariableValue(const std::vector<VariableValue> & v)         { _value = v; }
      VariableValue(const std::map<std::string, std::string> & v) { _value = v; }
      ~VariableValue() {}

      VariableValue &operator=(const VariableValue & v) {
        this->_value    = v._value;
        return *this;
      }

      bool operator==(const VariableValue& rhs) const {
        return boost::apply_visitor(EqualityVisitor(), this->_value, rhs._value);
      }

      template <typename T>
      T &as() {
        T &ret = boost::get<T> (_value);
        return ret;
      }

      template <typename T>
      const T &as() const {
        const T &ret = boost::get<T> (_value);
        return ret;
      }

      operator bool                       () const { return as<bool>(); }
      operator int                        () const { return as<int>(); }
      operator float                      () const { return as<float>(); }
      operator double                     () const { return as<double>(); }
      operator std::string                () const { return as<std::string>(); }
      operator std::vector<unsigned char> () const { return as<std::vector<unsigned char> >(); }
      operator std::vector<VariableValue> () const { return as<std::vector<VariableValue> >(); }
      operator std::map<std::string, std::string> () const { return as<std::map<std::string, std::string> >(); }

      /** Return the value as an VariableValue */
      const ValueType &value() const      { return _value; }
      ValueType &value()                  { return _value; }
      void value(const ValueType & value) { _value = value; }

      bool empty()const {
        return _value.type() == typeid(EmptyValue);
      }

    private:
      ValueType _value;
    };

    typedef std::vector<VariableValue> ArgumentList;
    typedef VariableValue              ReturnValue;
  }
}



#include <alcommon-ng/collections/print_visitor.hpp>
/**
 * Print the value in ostr stream.
 * @param ostr The stream to send the printed value in.
 * @param value The value to print.
 * @return ostr.
 */
inline std::ostream & operator << (std::ostream & ostr, const AL::Messaging::VariableValue & value) {
        AL::Messaging::PrintVisitor visitor(ostr);
        return boost::apply_visitor(visitor, value.value());
}

#endif /* !LIBIPPC_VARIABLESLIST_HPP_ */
