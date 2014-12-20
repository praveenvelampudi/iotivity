//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

package OC.Cm.Connector.Server.HTTPS;

import OC.Cm.Connector.IConnectorConfiguration;
import OC.Cm.Connector.SSLConfiguration;
import OC.Cm.Connector.Server.NativeServerConnector;

public class CMHTTPSServerConnector extends NativeServerConnector {

    private static native long constructNative(String ipaddress, String port);

    public CMHTTPSServerConnector(String ipaddress, String port) {
        super(constructNative(ipaddress, port));
    }

    @Override
    public IConnectorConfiguration getConnectorConfiguration() {
        return new SSLConfiguration(
                super.getConnectorConfiguration(mNativeHandle));
    }
}