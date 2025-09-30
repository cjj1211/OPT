#include "CMainFrameService.h"

#include "NxDBManager/CIdentifyInfoRepository.h"
#include "NxDBManager/CPatientRepository.h"
#include "NxDBManager/CUserRepository.h"

BEGIN_NX_NAMESPACE

CMainFrameService::CMainFrameService(QObject* parent)
    : IAppService(parent)
    , patientRepository(new CPatientRepository())
    , identityInfoRepository(new CIdentifyInfoRepository())
    , userRepository(new CUserRepository())
{
}

CMainFrameService::~CMainFrameService() = default;

User CMainFrameService::getUserByIdentityFK(const QString& identityFK) const
{
    auto users = userRepository->getUserByIdentifyFK(identityFK);
    if(users.empty())
    {
        return {};
    }
    else
    {
        return users[0];
    }
}

IdentityInfo CMainFrameService::getIdentityByUid(const QString& uid) const
{
    return identityInfoRepository->getIdentityInfoByUid(uid);
}

bool CMainFrameService::getUserByUid(const QString& uid, User& user) const
{
    return userRepository->GetByUID(uid, user);
}

bool CMainFrameService::getPatientByUid(const QString& uid, Patient& patient) const
{
    return patientRepository->GetByUID(uid, patient);
}

END_NX_NAMESPACE
