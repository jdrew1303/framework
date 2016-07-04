#pragma once
#include <samchon/API.hpp>

#include <samchon/protocol/SharedEntityDeque.hpp>
#	include <samchon/protocol/ExternalSystem.hpp>
#include <samchon/protocol/Server.hpp>
#include <samchon/protocol/IProtocol.hpp>

#include <samchon/protocol/ClientDriver.hpp>

#include <thread>

namespace samchon
{
namespace protocol
{
	class ExternalSystemArrayServer;

	class ExternalSystemArray
		: public SharedEntityDeque<ExternalSystem>,
		public virtual Server, // YOU CAN REPLACE IT -> WebServer
		public virtual IProtocol
	{
		friend class ExternalSystem;
		friend class ExternalSystemArrayServer;

	private:
		typedef SharedEntityDeque<ExternalSystem> super;

	public:
		/* ---------------------------------------------------------
			CONSTRUCTORS
				- DEFAULT
				- FACTORY METHOD FOR SERVER
		--------------------------------------------------------- */
		/**
		 * Default Constructor.
		 */
		ExternalSystemArray()
			: super(),
			Server()
		{
		};
		virtual ~ExternalSystemArray() = default;

	protected:
		virtual void addClient(std::shared_ptr<ClientDriver> driver)
		{
			ExternalSystem *system = createClient(driver);
			if (system == nullptr)
				return;

			system->communicator = driver;

			emplace_back(system);
			driver->listen(system);
		};

		virtual auto createClient(std::shared_ptr<ClientDriver> driver) -> ExternalSystem* = 0;
		
	public:
		/* ---------------------------------------------------------
			ACCESSORS
		--------------------------------------------------------- */
		auto hasRole(const std::string &key) const -> bool
		{
			for (size_t i = 0; i < size(); i++)
				for (size_t j = 0; j < at(i)->size(); j++)
					if (at(i)->at(j)->key() == key)
						return true;

			return false;
		};

		auto getRole(const std::string &key) const -> std::shared_ptr<ExternalSystemRole>
		{
			for (size_t i = 0; i < size(); i++)
				for (size_t j = 0; j < at(i)->size(); j++)
					if (at(i)->at(j)->key() == key)
						return at(i)->at(j);

			throw std::out_of_range("No such key.");
		};

		/* =========================================================
			NETWORK
				- SERVER AND CLIENT
				- MESSAGE CHAIN
		============================================================
			SERVER AND CLIENT
		--------------------------------------------------------- */
		virtual void open(int port) override
		{
			Server::open(port);
		};

		virtual void connect()
		{
			for (size_t i = 0; i < size(); i++)
				if (at(i)->communicator == nullptr && at(i)->ip.empty() == false)
					std::thread(&ExternalSystem::connect, at(i).get()).detach();
		};

		/* ---------------------------------------------------------
			MESSAGE CHAIN
		--------------------------------------------------------- */
		virtual void sendData(std::shared_ptr<Invoke> invoke)
		{
			for (size_t i = 0; i < size(); i++)
				at(i)->sendData(invoke);
		};

		virtual void replyData(std::shared_ptr<Invoke> invoke)
		{
			for (size_t i = 0; i < size(); i++)
				at(i)->replyData(invoke);
		};

		/* ---------------------------------------------------------
			EXPORTERS
		--------------------------------------------------------- */
		virtual auto TAG() const -> std::string override
		{
			return "systemArray";
		};

		virtual auto CHILD_TAG() const -> std::string override
		{
			return "system";
		};
	};
};
};