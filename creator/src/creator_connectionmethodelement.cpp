/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  
*
*/


#include "creator_connectionmethodelement.h"
#include "creator_traces.h"

#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
#include "creator_connectionmethod.h"
#include <cmpluginpacketdatadef.h>
#include <cmplugindialcommondefs.h>
#include <cmplugincsddef.h>
#include <cmpluginhscsddef.h>
#include <cmpluginembdestinationdef.h>
#include <cmpluginvpndef.h>
#include <cmpluginlanbasedef.h>
#else
#include "creator_accesspoint.h"
#endif


using namespace creatorconnectionmethod;

// connection name needs extra space for possible renaming
const TInt KExtraSpace = 10;

/*
 * 
 */
CCreatorConnectionMethodElement* CCreatorConnectionMethodElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext )
    {
    CCreatorConnectionMethodElement* self = new (ELeave) CCreatorConnectionMethodElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop(self);
    return self;
    }
/*
 * 
 */
CCreatorConnectionMethodElement::CCreatorConnectionMethodElement(CCreatorEngine* aEngine) 
: 
CCreatorScriptElement(aEngine)
    {
    iIsCommandElement = ETrue;
    }

#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
void CCreatorConnectionMethodElement::ExecuteCommandL()
    {
    const CCreatorScriptAttribute* amountAttr = FindAttributeByName(KAmount);
    TInt cmAmount = 1;    
    if( amountAttr )
        {
        cmAmount = ConvertStrToIntL(amountAttr->Value());
        }
    // Get 'fields' element 
    CCreatorScriptElement* fieldsElement = FindSubElement(KFields);
    if( fieldsElement && fieldsElement->SubElements().Count() > 0 )
        {
        // Get sub-elements
        const RPointerArray<CCreatorScriptElement>& fields = fieldsElement->SubElements();        
        // Create connection method entries, the amount of entries is defined by cmAmount:
        for( TInt cI = 0; cI < cmAmount; ++cI )
            {            
            CConnectionSettingsParameters* param = new (ELeave) CConnectionSettingsParameters;
            CleanupStack::PushL(param);
            
            for( TInt i = 0; i < fields.Count(); ++i )
                {
                CCreatorScriptElement* field = fields[i];
                TPtrC elemName = field->Name();
                TPtrC elemContent = field->Content();
                const CCreatorScriptAttribute* randomAttr = fields[i]->FindAttributeByName(KRandomLength);
                TBool useRandom = EFalse;
                if( randomAttr || elemContent.Length() == 0 )
                    {
                    useRandom = ETrue;
                    }
                
                if( elemName == Kconnectionname )
                    {
                    if( useRandom )
                        {
                        param->SetRandomCMNameL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iConnectionName, elemContent);
                        // reserve extra space for possible renaming
                        TInt newLen = param->iConnectionName->Length() + KExtraSpace;
                        param->iConnectionName = param->iConnectionName->ReAllocL(newLen);
                        }
                    }
                else if( elemName == Kstartpage )
                    {
                    if( useRandom )
                        {
                        param->SetRandomStartPageL(*iEngine);
                        }
                    else
                        {
                        SetContentToTextParamL(param->iStartPage, elemContent);
                        }
                    }
                else if( elemName == Kwapwspoption )
                    {
                    if( useRandom )
                        {
                        param->SetRandomWapWspOptionL(*iEngine);
                        }
                    else
                        {
                        if( CompareIgnoreCase(elemContent, Kconnectionless) == 0 )
                        	{
                        	param->iWapWspOption = ECmWapWspOptionConnectionless;
                        	}
                        else if( CompareIgnoreCase(elemContent, Kconnectionoriented) == 0 )
                        	{
                        	param->iWapWspOption = ECmWapWspOptionConnectionOriented;
                        	}
                        }
                    }
                else if( elemName == Kprotocoltype )
                	{
                	if( useRandom )
                		{
                		param->SetRandomProtocolTypeL(*iEngine);
                		}
                	else
                		{
                		if( CompareIgnoreCase(elemContent, Kipv4) == 0 )
                			{
                			param->iProtocolType = RPacketContext::EPdpTypeIPv4;
                			}
                		else if( CompareIgnoreCase(elemContent, Kipv6) == 0 )
                			{
                			param->iProtocolType = RPacketContext::EPdpTypeIPv6;
                			}
                		}
                	}
                else if( elemName == Kloginname )
                	{
                	if( useRandom )
                		{
                		param->SetRandomLoginNameL(*iEngine);                		
                		}
                	else
                		{
                		SetContentToTextParamL(param->iLoginName, elemContent);
                		}
                	}
                else if( elemName == Ksecureauthentication )
                	{
                	if( useRandom )
                		{
                		param->SetRandomSecureAuthenticationL(*iEngine);
                		}
                	else
                		{
                		param->iSecureAuthentication = ConvertStrToBooleanL(elemContent);
                		}
                	}
                else if( elemName == Kloginpass )
                	{
                	if( useRandom )
                		{
                		param->SetRandomLoginPassL(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iLoginPass, elemContent);
                		}
                	}
                else if( elemName == Kpromptpassword )
                	{
                	if( useRandom )
                		{
                		param->SetRandomPromptPasswordL(*iEngine);
                		}
                	else
                		{
                		param->iPromptPassword = ConvertStrToBooleanL(elemContent);
                		}
                	}
                else if( elemName == Kgatewayaddress )
                	{
                	if( useRandom )
                		{
                		param->SetRandomGatewayAddressL(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iGatewayAddress, elemContent);
                		}
                	}
                else if( elemName == Ksubnetmask )
                	{
                	if( useRandom )
                		{
                		param->SetRandomSubnetMaskL(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iSubnetMask, elemContent);
                		}
                	}
                else if( elemName == Kdeviceipaddr )
                	{
                	if( useRandom )
                		{
                		param->SetRandomIPAddressL(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iIPAddr, elemContent);
                		}
                	}
                else if( elemName == Kip4nameserver1 )
                	{
                	if( useRandom )
                		{
                		param->SetRandomIP4NameServer1L(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iIP4NameServer1, elemContent);
                		}
                	}
                else if( elemName == Kip4nameserver2 )
                	{
                	if( useRandom )
                		{
                		param->SetRandomIP4NameServer2L(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iIP4NameServer2, elemContent);
                		}
                	}
                else if( elemName == Kdatacalltelnumber )
                	{
                	if( useRandom )
                		{
                		param->SetRandomTelephoneNumberL(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iDefaultTelNumber, elemContent);
                		}
                	}
                else if( elemName == Kdatacalltypeisdn )
                	{
                	if( useRandom )
                		{
                		param->SetRandomBearerTypeIsdnL(*iEngine);
                		}
                	else
                		{
                		if( CompareIgnoreCase(elemContent, Kanalogue) == 0 )
                			param->iBearerCallTypeIsdn = CMManager::ECmCallTypeAnalogue;
                		else if( CompareIgnoreCase( elemContent, Kisdnv110) == 0 )
                			param->iBearerCallTypeIsdn = CMManager::ECmCallTypeISDNv110;
                		else if( CompareIgnoreCase( elemContent, Kisdnv120) == 0 )
                			param->iBearerCallTypeIsdn = CMManager::ECmCallTypeISDNv120;
                		}
                	}
                else if( elemName == Kbearertype )
                	{
                	if( useRandom )
                		{
                		param->iBearerType = KRandomBearerType;
                		}
                	else
                		{
                		if( CompareIgnoreCase(elemContent, Kwlan) == 0 )
                			{
                			param->iBearerType = KUidWlanBearerType;
                			}
                		else if( CompareIgnoreCase(elemContent, Kgprs) == 0 )
                			{
                			param->iBearerType = KUidPacketDataBearerType;
                			}
                		else if( CompareIgnoreCase(elemContent, Kdatacall) == 0 )
                			{
                			param->iBearerType = KUidCSDBearerType;
                			}
                		else if( CompareIgnoreCase(elemContent, Khsgsm) == 0 )
                			{
                			param->iBearerType = KUidHSCSDBearerType;
                			}
                		else if( CompareIgnoreCase(elemContent, Kembedded) == 0 )
                			{
                			param->iBearerType = KUidEmbeddedDestination;
                			}
                		else if( CompareIgnoreCase(elemContent, Kvpn) == 0 )
                			{
                			param->iBearerType = KPluginVPNBearerTypeUid;
                			}
                		else if( CompareIgnoreCase(elemContent, Klan) == 0 )
                			{
                			param->iBearerType = KUidLanBearerType;
                			}
                		}
                	}
                else if( elemName == Kdatacalllinespeed )
                	{
                	if( useRandom )
                		{
                		param->SetRandomBearerSpeedL(*iEngine);
                		}
                	else
                		{
                		if( CompareIgnoreCase(elemContent, Kautomatic) == 0 )
                			{                			 
                			param->iBearerSpeed = CMManager::ECmSpeedAutobaud;
                			}
                		else
                			{
                			TUint lineSpeed = ConvertStrToUintL(elemContent);
                			if( lineSpeed == 9600 )
                				param->iBearerSpeed = CMManager::ECmSpeed9600;
                			else if( lineSpeed == 14400 )
                				param->iBearerSpeed = CMManager::ECmSpeed14400;
                			else if( lineSpeed == 19200 )
                				param->iBearerSpeed = CMManager::ECmSpeed19200;
                			else if( lineSpeed == 28800 )
                				param->iBearerSpeed = CMManager::ECmSpeed28800;
                			else if( lineSpeed == 38400 )
                				param->iBearerSpeed = CMManager::ECmSpeed38400;
                			else if( lineSpeed == 43200 )
                				param->iBearerSpeed = CMManager::ECmSpeed43200;
                			else if( lineSpeed == 56000 )
                				param->iBearerSpeed = CMManager::ECmSpeed56000;
                			}                			
                		}
                	}
                else if( elemName == Kuseproxy )
                	{
                	if( useRandom )
                		{
                		param->SetRandomUseProxyL(*iEngine);
                		}
                	else
                		{
                		param->iUseProxy = ConvertStrToBooleanL(elemContent);
                		}
                	}
                else if( elemName == Kproxyserveraddress )
                	{
                	if( useRandom )
                		{
                		param->SetRandomProxyAddressL(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iProxyServerAddress, elemContent);                		
                		}
                	}
                else if( elemName == Kproxyportnumber )
                	{
                	if( useRandom )
                		{
                		param->SetRandomProxyPortL(*iEngine);
                		}
                	else
                		{
                		param->iProxyPortNumber = ConvertStrToUintL(elemContent);                		
                		}
                	}
                else if( elemName == Kip6nameserver1 )
                	{
                	if( useRandom )
                		{
                		param->SetRandomIPv6NameServer1L(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iIP6NameServer1, elemContent);               		                		
                		}
                	}
                else if( elemName == Kip6nameserver2 )
                	{
                	if( useRandom )
                		{
                		param->SetRandomIPv6NameServer2L(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iIP6NameServer2, elemContent);               		                		
                		}
                	}
                else if( elemName == Kdisabletextauth )
                	{
                	if( useRandom )
                		{
                		param->SetRandomDisableTextAuthL(*iEngine);
                		}
                	else
                		{
                		param->iDisableTextAuth = ConvertStrToBooleanL(elemContent);
                		}
                	}
                else if( elemName == Kwlanname )
                	{
                	if( useRandom )
                		{
                		param->SetRandomWLANNameL(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iWLANName, elemContent);               		                		
                		}
                	}
                else if( elemName == Kwlanipaddr )
                	{
                	if( useRandom )
                		{
                		param->SetRandomWlanIpAddrL(*iEngine);
                		}
                	else
                		{
                		SetContentToTextParamL(param->iWlanIpAddr, elemContent);               		                		
                		}
                	}
                else if( elemName == Kwlansecmode )
                	{
                	if( useRandom )
                		{
                		param->SetRandomWLANSecurityModeL(*iEngine);
                		}
                	else
                		{
                		if( CompareIgnoreCase(elemContent, Kopen) == 0)
                			param->iWLanSecMode = CMManager::EWlanSecModeOpen;
                		else if( CompareIgnoreCase(elemContent, Kwep) == 0 )
                			param->iWLanSecMode = CMManager::EWlanSecModeWep;
                		else if( CompareIgnoreCase(elemContent, Ke802_1x) == 0 )
                			param->iWLanSecMode = CMManager::EWlanSecMode802_1x;
                		else if( CompareIgnoreCase(elemContent, Kwpa) == 0 )
                			param->iWLanSecMode = CMManager::EWlanSecModeWpa;
                		else if( CompareIgnoreCase(elemContent, Kwpa2) == 0 )
                			param->iWLanSecMode = CMManager::EWlanSecModeWpa2;                		               		                		
                		}
                	}
                }
            iEngine->AppendToCommandArrayL(ECmdCreateMiscEntryAccessPoints, param);
            CleanupStack::Pop(); // param
            }
        }
    else
    	{
    	// No fields defined so add random entries:
    	iEngine->AppendToCommandArrayL(ECmdCreateMiscEntryAccessPoints, 0, cmAmount);
    	}
    }
#else
void CCreatorConnectionMethodElement::ExecuteCommandL()
	{
	const CCreatorScriptAttribute* amountAttr = FindAttributeByName(KAmount);
	TInt cmAmount = 1;    
	if( amountAttr )
		{
		cmAmount = ConvertStrToIntL(amountAttr->Value());
		}
	// Get 'fields' element 
	CCreatorScriptElement* fieldsElement = FindSubElement(KFields);
	if( fieldsElement )
		{
		// Get sub-elements
		const RPointerArray<CCreatorScriptElement>& fields = fieldsElement->SubElements();        
		// Create connection method entries, the amount of entries is defined by cmAmount:
		for( TInt cI = 0; cI < cmAmount; ++cI )
			{            
			CAccessPointsParameters* param = new (ELeave) CAccessPointsParameters;
			CleanupStack::PushL(param);

			for( TInt i = 0; i < fields.Count(); ++i )
				{
				CCreatorScriptElement* field = fields[i];
				TPtrC elemName = field->Name();
				TPtrC elemContent = field->Content();
				const CCreatorScriptAttribute* randomAttr = fields[i]->FindAttributeByName(KRandomLength);
				TBool useRandom = EFalse;
				if( randomAttr || elemContent.Length() == 0 )
					{
					useRandom = ETrue;
					}

				if( elemName == Kconnectionname )
					{
					if( useRandom )
						{
						param->SetRandomCMNameL(*iEngine);
						}
					else
						{
						SetContentToTextParamL(param->iConnectionName, elemContent);
                        // reserve extra space for possible renaming
                        TInt newLen = param->iConnectionName->Length() + KExtraSpace;
                        param->iConnectionName = param->iConnectionName->ReAllocL(newLen);

                        SetContentToTextParamL(param->iGprsAcessPointName, elemContent);
						}
					}
				else if( elemName == Kstartpage )
					{
					if( useRandom )
						{
						param->SetRandomStartPageL(*iEngine);
						}
					else
						{
						SetContentToTextParamL(param->iWapStartPage, elemContent);
						}
					}
				else if( elemName == Kwapwspoption )
					{
					if( useRandom )
						{
						param->SetRandomWapWspOptionL(*iEngine);
						}
					else
						{
						if( CompareIgnoreCase(elemContent, Kconnectionless) == 0 )
							{
							param->iWapWspOption = EWapWspOptionConnectionless;
							}
						else if( CompareIgnoreCase(elemContent, Kconnectionoriented) == 0 )
							{
							param->iWapWspOption = EWapWspOptionConnectionOriented;
							}
						}
					}
				else if( elemName == Kloginname )
					{
					if( useRandom )
						{
						param->SetRandomLoginNameL(*iEngine);                		
						}
					else
						{
						SetContentToTextParamL(param->iIspLoginName, elemContent);
						}
					}
				else if( elemName == Ksecureauthentication )
					{
					if( useRandom )
						{
						param->SetRandomSecureAuthenticationL(*iEngine);
						}
					else
						{
						param->iSecureAuthentication = ConvertStrToBooleanL(elemContent);
						}
					}
				else if( elemName == Kloginpass )
					{
					if( useRandom )
						{
						param->SetRandomLoginPassL(*iEngine);
						}
					else
						{
						SetContentToTextParamL(param->iIspLoginPass, elemContent);
						}
					}
				else if( elemName == Kpromptpassword )
					{
					if( useRandom )
						{
						param->SetRandomPromptPasswordL(*iEngine);
						}
					else
						{
						param->iPromptPassword = ConvertStrToBooleanL(elemContent);
						}
					}
				else if( elemName == Kgatewayaddress )
					{
					if( useRandom )
						{
						param->SetRandomGatewayAddressL(*iEngine);
						}
					else
						{
						SetContentToTextParamL(param->iWapGatewayAddress, elemContent);
						}
					}
				else if( elemName == Kdeviceipaddr )
					{
					if( useRandom )
						{
						param->SetRandomIPAddressL(*iEngine);
						}
					else
						{
						SetContentToTextParamL(param->iIspIPAddr, elemContent);
						}
					}
				else if( elemName == Kip4nameserver1 )
					{
					if( useRandom )
						{
						param->SetRandomIP4NameServer1L(*iEngine);
						}
					else
						{
						SetContentToTextParamL(param->iIspIPNameServer1, elemContent);
						}
					}
				else if( elemName == Kip4nameserver2 )
					{
					if( useRandom )
						{
						param->SetRandomIP4NameServer2L(*iEngine);
						}
					else
						{
						SetContentToTextParamL(param->iIspIPNameServer2, elemContent);
						}
					}
				else if( elemName == Kdatacalltelnumber )
					{
					if( useRandom )
						{
						param->SetRandomTelephoneNumberL(*iEngine);
						}
					else
						{
						SetContentToTextParamL(param->iIspDefaultTelNumber, elemContent);
						}
					}
				else if( elemName == Kdatacalltypeisdn )
					{
					if( useRandom )
						{
						param->SetRandomBearerTypeIsdnL(*iEngine);
						}
					else
						{
						if( CompareIgnoreCase(elemContent, Kanalogue) == 0 )
							param->iIspBearerCallTypeIsdn = ECallTypeAnalogue;
						else if( CompareIgnoreCase( elemContent, Kisdnv110) == 0 )
							param->iIspBearerCallTypeIsdn = ECallTypeISDNv110;
						else if( CompareIgnoreCase( elemContent, Kisdnv120) == 0 )
							param->iIspBearerCallTypeIsdn = ECallTypeISDNv120;
						}
					}
				else if( elemName == Kbearertype )
					{
					if( useRandom )
						{
						param->iBearerType = EApBearerTypeGPRS;
						}
					else
						{
						if( CompareIgnoreCase(elemContent, Kwlan) == 0 )
							{
							param->iBearerType = EApBearerTypeWLAN;
							}
						else if( CompareIgnoreCase(elemContent, Kgprs) == 0 )
							{
							param->iBearerType = EApBearerTypeGPRS;
							}
						else if( CompareIgnoreCase(elemContent, Kdatacall) == 0 )
							{
							param->iBearerType = EApBearerTypeCSD;
							}
						else if( CompareIgnoreCase(elemContent, Khsgsm) == 0 )
							{
							param->iBearerType = EApBearerTypeHSCSD;
							}
						else if( CompareIgnoreCase(elemContent, Klan) == 0 )
							{
							param->iBearerType = EApBearerTypeLAN;
							}
						}
					}
				else if( elemName == Kdatacalllinespeed )
					{
					if( useRandom )
						{
						param->SetRandomBearerSpeedL(*iEngine);
						}
					else
						{
						if( CompareIgnoreCase(elemContent, Kautomatic) == 0 )
							{                			 
							param->iIspBearerSpeed = KSpeedAutobaud;
							}
						else
							{
							TUint lineSpeed = ConvertStrToUintL(elemContent);
							if( lineSpeed == 9600 )
								param->iIspBearerSpeed = KSpeed9600;
							else if( lineSpeed == 14400 )
								param->iIspBearerSpeed = KSpeed14400;
							else if( lineSpeed == 19200 )
								param->iIspBearerSpeed = KSpeed19200;
							else if( lineSpeed == 28800 )
								param->iIspBearerSpeed = KSpeed28800;
							else if( lineSpeed == 38400 )
								param->iIspBearerSpeed = KSpeed38400;
							else if( lineSpeed == 43200 )
								param->iIspBearerSpeed = KSpeed43200;
							else if( lineSpeed == 56000 )
								param->iIspBearerSpeed = KSpeed56000;
							}                			
						}
					}
				else if( elemName == Kproxyserveraddress )
					{
					if( useRandom )
						{
						param->SetRandomProxyAddressL(*iEngine);
						}
					else
						{
						SetContentToTextParamL(param->iProxyServerAddress, elemContent);                		
						}
					}
				else if( elemName == Kproxyportnumber )
					{
					if( useRandom )
						{
						param->SetRandomProxyPortL(*iEngine);
						}
					else
						{
						param->iProxyPortNumber = ConvertStrToUintL(elemContent);                		
						}
					}
				}
			iEngine->AppendToCommandArrayL(ECmdCreateMiscEntryAccessPoints, param);
			CleanupStack::Pop(); // param
			}
		}
	}
#endif
