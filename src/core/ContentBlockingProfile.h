/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2010 - 2014 David Rosca <nowrep@gmail.com>
* Copyright (C) 2014 - 2017 Jan Bajer aka bajasoft <jbajer@gmail.com>
* Copyright (C) 2015 - 2018 Michal Dutkiewicz aka Emdek <michal@emdek.pl>

* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#ifndef OTTER_CONTENTBLOCKINGPROFILE_H
#define OTTER_CONTENTBLOCKINGPROFILE_H

#include "ContentBlockingManager.h"

#include <QtCore/QRegularExpression>

namespace Otter
{

class ContentBlockingProfile final : public QObject
{
	Q_OBJECT

public:
	enum ProfileError
	{
		NoError = 0,
		ReadError,
		DownloadError,
		ChecksumError
	};

	enum ProfileFlag
	{
		NoFlags = 0,
		HasCustomTitleFlag = 1,
		HasCustomUpdateUrlFlag = 2
	};

	Q_DECLARE_FLAGS(ProfileFlags, ProfileFlag)

	enum ProfileCategory
	{
		OtherCategory = 0,
		AdvertisementsCategory = 1,
		AnnoyanceCategory = 2,
		PrivacyCategory = 4,
		SocialCategory = 8,
		RegionalCategory = 16
	};

	explicit ContentBlockingProfile(const QString &name, const QString &title, const QUrl &updateUrl, const QDateTime &lastUpdate, const QStringList &languages, int updateInterval, const ProfileCategory &category, const ProfileFlags &flags, QObject *parent = nullptr);

	void clear();
	void setCategory(const ProfileCategory &category);
	void setTitle(const QString &title);
	void setUpdateInterval(int interval);
	void setUpdateUrl(const QUrl &url);
	QString getName() const;
	QString getTitle() const;
	QUrl getUpdateUrl() const;
	QDateTime getLastUpdate() const;
	ContentBlockingManager::CheckResult checkUrl(const QUrl &baseUrl, const QUrl &requestUrl, NetworkManager::ResourceType resourceType);
	ContentBlockingManager::CosmeticFiltersResult getCosmeticFilters(const QStringList &domains, bool isDomainOnly);
	QVector<QLocale::Language> getLanguages() const;
	ProfileCategory getCategory() const;
	ProfileError getError() const;
	ProfileFlags getFlags() const;
	int getUpdateInterval() const;
	bool downloadRules();
	bool remove();
	bool isUpdating() const;

protected:
	enum RuleOption : quint32
	{
		NoOption = 0,
		ThirdPartyOption = 1,
		ThirdPartyExceptionOption = 2,
		StyleSheetOption = 4,
		StyleSheetExceptionOption = 8,
		ScriptOption = 16,
		ScriptExceptionOption = 32,
		ImageOption = 64,
		ImageExceptionOption = 128,
		ObjectOption = 256,
		ObjectExceptionOption = 512,
		ObjectSubRequestOption = 1024,
		ObjectSubRequestExceptionOption = 2048,
		SubDocumentOption = 4096,
		SubDocumentExceptionOption = 8192,
		XmlHttpRequestOption = 16384,
		XmlHttpRequestExceptionOption = 32768,
		WebSocketOption = 65536,
		PopupOption = 131072,
		ElementHideOption = 262144,
		GenericHideOption = 524288
	};

	Q_DECLARE_FLAGS(RuleOptions, RuleOption)

	enum RuleMatch
	{
		ContainsMatch = 0,
		StartMatch,
		EndMatch,
		ExactMatch
	};

	struct ContentBlockingRule final
	{
		QString rule;
		QStringList blockedDomains;
		QStringList allowedDomains;
		RuleOptions ruleOptions = NoOption;
		RuleMatch ruleMatch = ContainsMatch;
		bool isException = false;
		bool needsDomainCheck = false;

		explicit ContentBlockingRule(QString ruleValue, QStringList blockedDomainsValue, QStringList allowedDomainsValue, RuleOptions ruleOptionsValue, RuleMatch ruleMatchValue, bool isExceptionValue, bool needsDomainCheckValue) : rule(ruleValue), blockedDomains(blockedDomainsValue), allowedDomains(allowedDomainsValue), ruleOptions(ruleOptionsValue), ruleMatch(ruleMatchValue), isException(isExceptionValue), needsDomainCheck(needsDomainCheckValue)
		{
		}
	};

	struct Node final
	{
		QChar value = 0;
		QVarLengthArray<Node*, 1> children;
		QVarLengthArray<ContentBlockingRule*, 1> rules;
	};

	QString getPath() const;
	void loadHeader(const QString &path);
	void parseRuleLine(const QString &rule);
	void parseStyleSheetRule(const QStringList &line, QMultiHash<QString, QString> &list) const;
	void addRule(ContentBlockingRule *rule, const QString &ruleString) const;
	void deleteNode(Node *node) const;
	ContentBlockingManager::CheckResult checkUrlSubstring(const Node *node, const QString &subString, QString currentRule, NetworkManager::ResourceType resourceType);
	ContentBlockingManager::CheckResult checkRuleMatch(const ContentBlockingRule *rule, const QString &currentRule, NetworkManager::ResourceType resourceType) const;
	ContentBlockingManager::CheckResult evaluateRulesInNode(const Node *node, const QString &currentRule, NetworkManager::ResourceType resourceType) const;
	bool loadRules();
	bool resolveDomainExceptions(const QString &url, const QStringList &ruleList) const;

protected slots:
	void handleReplyFinished();

private:
	Node *m_root;
	QNetworkReply *m_networkReply;
	QString m_requestUrl;
	QString m_requestHost;
	QString m_baseUrlHost;
	QString m_name;
	QString m_title;
	QUrl m_updateUrl;
	QDateTime m_lastUpdate;
	QRegularExpression m_domainExpression;
	QStringList m_cosmeticFiltersRules;
	QVector<QLocale::Language> m_languages;
	QMultiHash<QString, QString> m_cosmeticFiltersDomainRules;
	QMultiHash<QString, QString> m_cosmeticFiltersDomainExceptions;
	ProfileCategory m_category;
	ProfileError m_error;
	ProfileFlags m_flags;
	int m_updateInterval;
	bool m_isUpdating;
	bool m_isEmpty;
	bool m_wasLoaded;

	static QVector<QChar> m_separators;
	static QHash<QString, RuleOption> m_options;
	static QHash<NetworkManager::ResourceType, RuleOption> m_resourceTypes;

signals:
	void profileModified(const QString &profile);
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Otter::ContentBlockingProfile::ProfileFlags)

#endif
