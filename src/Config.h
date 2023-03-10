//
// Created by liangzhuohua on 2023/3/10.
//

#ifndef TALUSTUN_CONFIG_H
#define TALUSTUN_CONFIG_H


#define TUN_CONFIG "talusTun."
#define CONFIG_MODE TUN_CONFIG"mode"
#define CONFIG_NAME TUN_CONFIG"name"


#define SERVER_CONFIG "talusTunServer."
#define CONFIG_WS_LISTEN_IP SERVER_CONFIG"wsListenIp"
#define CONFIG_WS_LISTEN_PORT SERVER_CONFIG"wsListenPort"
#define CONFIG_TUN_NET_IP SERVER_CONFIG"tunNetIp"
#define CONFIG_TUN_NET_MASK SERVER_CONFIG"tunNetMask"
#define CONFIG_TUN_NET_MTU SERVER_CONFIG"tunNetMtu"
#define CONFIG_SSL_CERT SERVER_CONFIG"sslCert"
#define CONFIG_ADDR_ALLOC_BEGIN SERVER_CONFIG"addrAllocBegin"
#define CONFIG_ADDR_ALLOC_END SERVER_CONFIG"addrAllocEnd"
#define CONFIG_ENABLE_SSL SERVER_CONFIG"enableSsl"


#define CLIENT_CONFIG "talusTunClient."
#define CONFIG_WS_URL CLIENT_CONFIG"wsUrl"


#endif //TALUSTUN_CONFIG_H
