/*
* This source file is part of ArkGameFrame
* For the latest info, see https://github.com/ArkGame
*
* Copyright (c) 2013-2018 ArkGame authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include "Example3Module.h"
#include "SDK/Proto/ARKDataDefine.hpp"
#include "SDK/Core/Base/AFTime.hpp"

bool Example3Module::Init()
{
    std::cout << typeid(Example3Module).name() << ", Init" << std::endl;
    return true;
}

int Example3Module::OnEvent(const AFGUID& self, const int event, const AFIDataList& arg)
{
    //事件回调函数
    std::cout << "OnEvent EventID: " << event << " self: " << self.ToString() << " argList: " << arg.String(0) << " " << " " << arg.Int(1) << std::endl;

    m_pKernelModule->SetNodeString(self, "Hello", arg.String(0));
    m_pKernelModule->SetNodeInt(self, "World", arg.Int(1));

    return 0;
}

int Example3Module::OnHeartBeat(const AFGUID& self, const std::string& strHeartBeat, const int64_t nTime, const int nCount)
{
    int64_t unNowTime = AFCTimeBase::GetInstance().GetNowMillisecond();
    std::cout << "strHeartBeat: " << nTime << " Count: " << nCount << "  TimeDis: " << unNowTime - mLastTime << std::endl;

    mLastTime = unNowTime;

    return 0;
}

int Example3Module::OnClassCallBackEvent(const AFGUID& self, const std::string& strClassName, const ARK_ENTITY_EVENT event, const AFIDataList& arg)
{
    //虚拟类事件，只要有此虚拟类创建或者销毁即会回调
    std::cout << "OnClassCallBackEvent ClassName: " << strClassName << " ID: " << self.ToString() << " Event: " << event << std::endl;

    if(event == ARK_ENTITY_EVENT::ENTITY_EVT_DATA_FINISHED)
    {
        m_pKernelModule->AddEventCallBack(self, 11111111, this, &Example3Module::OnEvent);

        m_pKernelModule->AddHeartBeat(self, "OnHeartBeat", this, &Example3Module::OnHeartBeat, 1000, 1, true);

        mLastTime = AFMisc::GetSystemTime();
    }

    return 0;
}

int Example3Module::OnIntDataNodeCB(const AFGUID& self, const std::string& strProperty, const AFIData& oldVar, const AFIData& newVar)
{
    //属性回调事件，只要属性值内容有变化，就会被回调
    std::cout << "OnPropertyCallBackEvent Property: " << strProperty << " OldValue: " << oldVar.GetInt() << " NewValue: " << newVar.GetInt() << std::endl;

    return 0;
}

int Example3Module::OnStrDataNodeCB(const AFGUID& self, const std::string& strProperty, const AFIData& oldVar, const AFIData& newVar)
{
    //属性回调事件，只要属性值内容有变化，就会被回调
    std::cout << "OnPropertyCallBackEvent Property: " << strProperty << " OldValue: " << oldVar.GetString() << " NewValue: " << newVar.GetString() << std::endl;

    return 0;
}

int Example3Module::OnFightHeroTableCB(const AFGUID& self, const DATA_TABLE_EVENT_DATA& table_data, const AFIData& old_data, const AFIData& new_data)
{
    if(table_data.nCol == ARK::Player::PlayerFightHero::PlayerFightHero_FightPos)
    {
        std::cout << "OnFightHeroTableCB, table_name = " << table_data.strName.c_str() << " old_data = " << old_data.GetInt() << " new_data = " << new_data.GetInt();
    }

    return 0;
}

bool Example3Module::PostInit()
{
    std::cout << typeid(Example3Module).name() << ", PostInit" << std::endl;

    m_pKernelModule = pPluginManager->FindModule<AFIKernelModule>();
    m_pElementModule = pPluginManager->FindModule<AFIElementModule>();
    m_pLogModule = pPluginManager->FindModule<AFILogModule>();

    ARK_LOG_INFO("Init finished...");

    //Create scene, all entity need in scene
    m_pKernelModule->CreateScene(1);

    //Add Class callback
    m_pKernelModule->AddClassCallBack(ARK::Player::ThisName(), this, &Example3Module::OnClassCallBackEvent);

    //Create Entity
    //Entity GUID is (0, 10)
    //SceneID = 1
    //GroupID = 0
    //Class Name = ARK::Player::ThisName()
    ARK_SHARE_PTR<AFIEntity> pEntity = m_pKernelModule->CreateEntity(AFGUID(0, 10), 1, 0, ARK::Player::ThisName(), "", AFCDataList());
    if(nullptr == pEntity)
    {
        return false;
    }

    AFCData xData;
    pEntity->GetNodeManager()->AddNode("str_test", AFCData(DT_STRING, ""), 0);
    pEntity->GetNodeManager()->AddNode("int_test", AFCData(DT_INT, 1), 0);

    pEntity->AddNodeCallBack("str_test", this, &Example3Module::OnStrDataNodeCB);
    pEntity->AddNodeCallBack("int_test", this, &Example3Module::OnIntDataNodeCB);

    pEntity->SetNodeString("str_test", "hello World");
    pEntity->SetNodeInt("int_test", 1111);

    pEntity->AddTableCallBack(ARK::Player::R_PlayerFightHero(), this, &Example3Module::OnFightHeroTableCB);

    AFDataTable* pTable = m_pKernelModule->FindTable(pEntity->Self(), ARK::Player::R_PlayerFightHero());
    int pos = 0;
    if(pTable != nullptr)
    {
        pTable->AddRow(-1, AFCDataList() << AFGUID(0, 1000) << 1);

        m_pKernelModule->SetTableInt(pEntity->Self(), ARK::Player::R_PlayerFightHero(), 0, ARK::Player::PlayerFightHero::PlayerFightHero_FightPos, 2);
        pos = m_pKernelModule->GetTableInt(pEntity->Self(), ARK::Player::R_PlayerFightHero(), 0, ARK::Player::PlayerFightHero::PlayerFightHero_FightPos);
    }

    m_pKernelModule->DoEvent(pEntity->Self(), 11111111, AFCDataList() << "another_test" << int(200));

    return true;
}

void Example3Module::Update()
{

}

bool Example3Module::PreShut()
{
    std::cout << typeid(Example3Module).name() << ", PreShut" << std::endl;
    m_pKernelModule->DestroyAll();

    return true;
}

bool Example3Module::Shut()
{
    std::cout << typeid(Example3Module).name() << ", Shut" << std::endl;
    return true;
}