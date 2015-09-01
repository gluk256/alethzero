/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file AlethZero.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#ifdef Q_MOC_RUN
#define BOOST_MPL_IF_HPP_INCLUDED
#endif

#include <map>
#include <QtNetwork/QNetworkAccessManager>
#include <QtCore/QAbstractListModel>
#include <QtCore/QMutex>
#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <libdevcore/RLP.h>
#include <libethcore/Common.h>
#include <libethcore/KeyManager.h>
#include <libethereum/State.h>
#include <libethereum/Executive.h>
#include <libwebthree/WebThree.h>
#include <libsolidity/CompilerStack.h>
#include "NatspecHandler.h"
#include "Connect.h"
#include "Aleth.h"

class QListWidgetItem;
class QActionGroup;

namespace Ui {
class Main;
}

namespace dev
{

class SafeHttpServer;

namespace eth
{
class Client;
class State;
}

namespace aleth
{

class OurWebThreeStubServer;
class DappLoader;
class DappHost;
struct Dapp;

class AlethZero: public Aleth
{
	Q_OBJECT

public:
	explicit AlethZero(QWidget *parent = 0);
	~AlethZero();

	WebThreeDirect* web3() const override { return m_webThree.get(); }
	OurWebThreeStubServer* web3Server() const override { return m_server.get(); }
	dev::SafeHttpServer* web3ServerConnector() const override { return m_httpConnector.get(); }
	eth::Client* ethereum() const override { return m_webThree->ethereum(); }
	std::shared_ptr<shh::WhisperHost> whisper() const override { return m_webThree->whisper(); }

	bool confirm() const;
	NatSpecFace* natSpec() override { return &m_natSpecDB; }

	std::string pretty(dev::Address const& _a) const override;
	std::string render(dev::Address const& _a) const override;
	std::pair<Address, bytes> fromString(std::string const& _a) const override;
	std::string renderDiff(eth::StateDiff const& _d) const override;

	eth::KeyManager& keyManager() override { return m_keyManager; }
	void noteKeysChanged() override { refreshBalances(); }
	bool doConfirm();

	Secret retrieveSecret(Address const& _address) const override;

	// Account naming API.
	void install(AccountNamer* _adopt) override;
	void uninstall(AccountNamer* _kill) override;
	void noteKnownAddressesChanged(AccountNamer*) override;
	void noteAddressNamesChanged(AccountNamer*) override;
	Address toAddress(std::string const&) const override;
	std::string toName(Address const&) const override;
	Addresses allKnownAddresses() const override;

	void noteSettingsChanged() override { writeSettings(); }

public slots:
	void note(QString _entry);
	void debug(QString _entry);
	void warn(QString _entry);
	QString contents(QString _file);

	int authenticate(QString _title, QString _text);

	void onKeysChanged();

private slots:
	// Application
	void on_about_triggered();
	void on_quit_triggered() { close(); }

	// Network
	void on_go_triggered();
	void on_net_triggered();
	void on_connect_triggered();
	void on_idealPeers_valueChanged(int);

	// Mining
	void on_mine_triggered();
	void on_prepNextDAG_triggered();

	// View
	void on_refresh_triggered();
	void on_preview_triggered();

	// Account management
	void on_killAccount_triggered();
	void on_reencryptKey_triggered();
	void on_reencryptAll_triggered();
	void on_exportKey_triggered();

	// Stuff concerning the blocks/transactions/accounts panels
	void on_ourAccounts_itemClicked(QListWidgetItem* _i);
	void on_ourAccounts_doubleClicked();

	// Special (debug) stuff
	void on_paranoia_triggered();
	void on_killBlockchain_triggered();
	void on_clearPending_triggered();
	void on_injectBlock_triggered();
	void on_forceMining_triggered();
	void on_usePrivate_triggered();
	void on_turboMining_triggered();
	void on_retryUnknown_triggered();
	void on_vmInterpreter_triggered();
	void on_vmJIT_triggered();
	void on_vmSmart_triggered();
	void on_rewindChain_triggered();

	// Config
	void on_sentinel_triggered();

signals:
	void poll();

private:
	template <class P> void loadPlugin() { Plugin* p = new P(this); initPlugin(p); }
	void initPlugin(Plugin* _p);
	void finalisePlugin(Plugin* _p);
	void unloadPlugin(std::string const& _name);

	p2p::NetworkPreferences netPrefs() const;

	QString lookup(QString const& _n) const;

	void updateFee();
	void readSettings(bool _skipGeometry = false, bool _onlyGeometry = false);
	void writeSettings();

	void setPrivateChain(QString const& _private, bool _forceConfigure = false);

	void keysChanged();

	void onNewBlock();
	void installWatches();

	virtual void timerEvent(QTimerEvent*) override;

	void refreshNetwork();
	void refreshMining();

	void refreshAll();
	void refreshBlockCount();
	void refreshBalances();

	void setBeneficiary(Address const& _b);
	std::string getPassword(std::string const& _title, std::string const& _for, std::string* _hint = nullptr, bool* _ok = nullptr);

	std::unique_ptr<Ui::Main> ui;
	std::unique_ptr<WebThreeDirect> m_webThree;

	QByteArray m_networkConfig;
	QStringList m_servers;
	eth::KeyManager m_keyManager;
	QString m_privateChain;
	dev::Address m_beneficiary;

	QActionGroup* m_vmSelectionGroup = nullptr;

	std::unique_ptr<dev::SafeHttpServer> m_httpConnector;
	std::unique_ptr<OurWebThreeStubServer> m_server;

	NatspecHandler m_natSpecDB;

	Connect m_connect;

	std::unordered_set<AccountNamer*> m_namers;

	bool m_destructing = false;
};

}
}