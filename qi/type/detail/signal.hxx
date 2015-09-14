#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_SIGNAL_HXX_
#define _QITYPE_DETAIL_SIGNAL_HXX_

#include <qi/trackable.hpp>
#include <qi/type/detail/manageable.hpp>
#include <boost/bind.hpp>
#include <qi/type/detail/functionsignature.hxx>

namespace qi
{
  template <typename T>
  template <typename Arg0, typename F, typename FFB>
  inline
      typename boost::enable_if<boost::is_base_of<Actor, typename detail::Unwrap<Arg0>::type>, SignalSubscriber&>::type
      SignalF<T>::_connectMaybeActor(Arg0& arg0, F&& cb, FFB&& fallbackCb)
  {
    SignalSubscriber& s =
        connect(qi::trackWithFallback(std::move(fallbackCb),
                                      boost::function<T>(
                                          detail::Unwrap<Arg0>::unwrap(arg0)->strand()->schedulerFor(std::move(cb))),
                                      arg0));
    // skip a bounce because we schedule on a strand, call will be async
    // anyway
    s.setCallType(MetaCallType_Direct);
    return s;
  }
  template <typename T>
  template <typename Arg0, typename F, typename FFB>
  inline typename boost::disable_if<
      boost::is_base_of<Actor, typename detail::Unwrap<Arg0>::type>,
      SignalSubscriber&>::type
      SignalF<T>::_connectMaybeActor(Arg0&& arg0,
                                     F&& cb,
                                     FFB&& fallbackCb)
  {
    return connect(qi::trackWithFallback(std::move(fallbackCb), std::move(cb), std::move(arg0)));
  }

  template <typename T>
  template <typename F, typename Arg0, typename... Args>
  SignalSubscriber& SignalF<T>::connect(F&& func, Arg0&& arg0, Args&&... args)
  {
    int curId;
    SignalLink* trackLink;
    createNewTrackLink(curId, trackLink);
    SignalSubscriber& s =
        _connectMaybeActor(arg0,
                           qi::bind<T>(func, arg0, args...),
                           qi::track(boost::function<void()>(
                                         // FIXME: i think this is unsafe, call the function on signalbaseprivate
                                         boost::bind(&SignalF<T>::disconnectTrackLink, this, curId)),
                                     boost::weak_ptr<SignalBasePrivate>(_p)));
    *trackLink = s;
    return s;
  }

  template<typename T>
  SignalSubscriber& SignalF<T>::connect(AnyFunction f)
  {
    return SignalBase::connect(SignalSubscriber(f));
  }
  template<typename T>
  SignalSubscriber& SignalF<T>::connect(const SignalSubscriber& sub)
  {
    return SignalBase::connect(sub);
  }
  template<typename T>
  template<typename U>
  SignalSubscriber&  SignalF<T>::connect(SignalF<U>& signal)
  {
    int curId;
    SignalLink* trackLink;
    createNewTrackLink(curId, trackLink);
    SignalSubscriber& s = connect(qi::trackWithFallback(
          boost::bind(&SignalF<T>::disconnectTrackLink, this, curId),
          (boost::function<U>&)signal,
          boost::weak_ptr<SignalBasePrivate>(signal._p)));
    *trackLink = s;
    return s;
  }

  template<typename T>
  template<QI_SIGNAL_TEMPLATE_DECL>
  SignalSubscriber&  SignalF<T>::connect(Signal<QI_SIGNAL_TEMPLATE>& signal)
  {
    typedef typename detail::VoidFunctionType<QI_SIGNAL_TEMPLATE>::type ftype;
    int curId;
    SignalLink* trackLink;
    createNewTrackLink(curId, trackLink);
    SignalSubscriber& s = connect(qi::trackWithFallback(
          boost::bind(&SignalF<T>::disconnectTrackLink, this, curId),
          (boost::function<ftype>&)signal,
          boost::weak_ptr<SignalBasePrivate>(signal._p)));
    *trackLink = s;
    return s;
  }

  template<typename F>
  SignalSubscriber& SignalBase::connect(const boost::function<F>& fun)
  {
    return connect(AnyFunction::from(fun));
  }
  template<typename T>
  template<typename F>
  SignalSubscriber& SignalF<T>::connect(F c)
  {
    return connect(qi::AnyFunction::from(boost::function<T>(std::move(c))));
  }
  template<typename T>
  SignalSubscriber& SignalF<T>::connect(const AnyObject& obj, const std::string& slot)
  {
    return SignalBase::connect(obj, slot);
  }

  template<typename T>
  SignalSubscriber& SignalF<T>::connect(const AnyObject& obj, unsigned int slot)
  {
    return connect(SignalSubscriber(obj, slot));
  }

  namespace detail
  {

  template<typename T> class BounceToSignalBase
  {
    // This default should not be instanciated
    static_assert(sizeof(T) < 0, "You can't instanciate BounceToSignalBase");
    public:
    BounceToSignalBase(SignalBase& sb)
    {
    }
  };
  #define pushArg(z, n, _) \
    args.push_back(AutoAnyReference(p ##n));
  #define makeBounce(n, argstypedecl, argstype, argsdecl, argsues, comma)     \
  template<typename R comma argstypedecl> \
  class BounceToSignalBase<R(argstype)>  {  \
  public:                      \
    BounceToSignalBase(SignalBase& signalBase) : signalBase(signalBase) {} \
    R operator()(argsdecl) {   \
      AnyReferenceVector args; \
      BOOST_PP_REPEAT(n, pushArg, _);    \
      signalBase.trigger(args);          \
    }                                    \
  private:                               \
    SignalBase& signalBase;              \
  };
  QI_GEN(makeBounce)
  #undef makeBounce
  #undef pushArg

  } // detail

  template<typename T>
  SignalF<T>::SignalF(OnSubscribers onSubscribers)
  : SignalBase(onSubscribers)
  {
    * (boost::function<T>*)this = detail::BounceToSignalBase<T>(*this);
    _setSignature(detail::functionArgumentsSignature<T>());
  }


  template<typename T>
  qi::Signature SignalF<T>::signature() const
  {
    return detail::functionArgumentsSignature<T>();
  }

  inline
  SignalSubscriber& SignalSubscriber::setCallType(MetaCallType ct)
  {
    threadingModel = ct;
    return *this;
  }
} // qi
#endif  // _QITYPE_DETAIL_SIGNAL_HXX_
