#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_SERVER_IMPL_HPP_
#define _QI_MESSAGING_SRC_SERVER_IMPL_HPP_

#include "src/messaging/serviceinfo.hpp"
#include "src/messaging/mutexednamelookup.hpp"
#include "src/messaging/impl_base.hpp"
//#include <qimessaging/transport/transport_message_handler.hpp>
#include <qimessaging/transport/transport_server.hpp>

namespace qi {
  namespace detail {
    class PublisherImpl;

    class ServerImpl :
      public qi::detail::ImplBase,
      public qi::TransportServerDelegate {
    public:
      virtual ~ServerImpl();

      ServerImpl(const std::string& name = "", Context *ctx = 0);
      void connect(const std::string& masterAddress = "127.0.0.1:5555");

      const std::string& getName() const;

      /// <summary>Advertises a service. </summary>
      /// <param name="methodSignature">The method signature.</param>
      /// <param name="functor">The functor that handles calls to the service.</param>
      void advertiseService(const std::string& methodSignature, qi::Functor* functor);

      /// <summary>Unadvertises a service. </summary>
      /// <param name="methodSignature">The method signature.</param>
      void unadvertiseService(const std::string& methodSignature);

      /// <summary>Message handler that implements the MessageHandler interface </summary>
      /// <param name="request">The serialized request message</param>
      /// <param name="reply">The serializaed reply message</param>
      void messageHandler(std::string& request, std::string& reply);
      // -----------------------------------------------

      virtual void onConnected(const std::string &msg = "") {};
      virtual void onWrite(const std::string &msg = "") {};
      virtual void onRead(const std::string &msg = "") {};


    protected:
      /// <summary> true if this server belongs to the master </summary>
      bool _isMasterServer;

      /// <summary> The underlying transport server </summary>
      qi::TransportServer _transportServer;

      /// <summary> A map from methodSignature to ServiceInfo </summary>
      MutexedNameLookup<ServiceInfo> _localServices;

      const ServiceInfo& xGetService(const std::string& signature);
      void xRegisterServiceWithMaster(const std::string& signature);
    };
  }
}

#endif  // _QI_MESSAGING_SRC_SERVER_IMPL_HPP_

