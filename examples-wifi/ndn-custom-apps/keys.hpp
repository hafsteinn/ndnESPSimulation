#ifndef KEYS_H_
#define KEYS_H_

#include <string>

namespace BlockNDN {

	class Keys
	{
        public:
            std::string getPrimaryPrivateKey(){return this->PRIMARYPrivateKey;};
            std::string getESPGhostPublicKey(){return this->ESPGhostPublicKey;};
            std::string getESPGhostPrivateKey(){return this->ESPGhostPrivateKey;};
            std::string getConsumerPublicKey(){return this->consumerPublicKey;};
            std::string getConsumerPrivateKey(){return this->consumerPrivateKey;};
            std::string getConsumerOnePublicKey(){return this->consumerOnePublicKey;};
            std::string getConsumerOnePrivateKey(){return this->consumerOnePrivateKey;};
            std::string getConsumerTwoPublicKey(){return this->consumerTwoPublicKey;};
            std::string getConsumerTwoPrivateKey(){return this->consumerTwoPrivateKey;};
            std::string getConsumerThreePublicKey(){return this->consumerThreePublicKey;};
            std::string getConsumerThreePrivateKey(){return this->consumerThreePrivateKey;};
            std::string getProducerPublicKey(){return this->producerPublicKey;};
            std::string getProducerPrivateKey(){return this->producerPrivateKey;};
        private:
            std::string producerPublicKey = "-----BEGIN PUBLIC KEY-----\n"\
            "MIGeMA0GCSqGSIb3DQEBAQUAA4GMADCBiAKBgHFAkdKwcJ/5lsuTnnJLNStf5PlV\n"\
            "dgFFaq6tVLv20/C4oMpA/lmD7tzXbV1pI1IVBR+w33u902TxlvfdGwPJIPbsqt6t\n"\
            "cqvpkkBQwKCgxlT4568x18AHY0cwcRhy9Hw6ff3iEp6lnZVLa7oge/IaysMm4z6e\n"\
            "voXgqPLJnUPBtx5VAgMBAAE=\n"\
            "-----END PUBLIC KEY-----\n\0";
            std::string producerPrivateKey = "-----BEGIN RSA PRIVATE KEY-----\n"\
            "MIICWgIBAAKBgHFAkdKwcJ/5lsuTnnJLNStf5PlVdgFFaq6tVLv20/C4oMpA/lmD\n"\
            "7tzXbV1pI1IVBR+w33u902TxlvfdGwPJIPbsqt6tcqvpkkBQwKCgxlT4568x18AH\n"\
            "Y0cwcRhy9Hw6ff3iEp6lnZVLa7oge/IaysMm4z6evoXgqPLJnUPBtx5VAgMBAAEC\n"\
            "gYAsFTwtzKwqORbvayiTyJ6LoaoZO6DRYdni4GlJOQzKeLNtxtq6sVY69EgHwjMT\n"\
            "ckcqTMjQ+9TbSAbBuHkZgiKoNTLw2n0mN5zRlkKrt/lGUfIzChOMhRe1OuHc/sFp\n"\
            "0WtPuoF30lP9go/FNi0CNt3UlPOfnroYDatTvGXdOiAyCQJBAMMNVwkUCEeuKFbe\n"\
            "KTnsj+0bqC4BXyqTvVIgiMTz7vup/p/jk62OWghhEG/29eKrk0PFiZLUok1tReLh\n"\
            "tWf2cfsCQQCUo98ip1PiaE509GjiX/+o8brnl5ybLXxQmwY10McuR1TCzdFZM/Ey\n"\
            "K2Z0JR/TXf5OFw7g/E5QECsblRrIzw/vAkAoWfwlfqVv3tGtluxEdXIZBV23lsUP\n"\
            "U90Yvm5NAIHyQmrQvQ3yG774eMwjvZ2EtSYUISui2qZxCigpFbSnu049AkBPG1PC\n"\
            "/qi/Q2IWDonPiOXoxRYC8YaQc8nIoGjStVV8RMxQjPdQ8Tr5x21UfPmOJWUVrGHn\n"\
            "zX/kTmaS3G0P2FuTAkAJiTV7VxopNl7qJXLvMPZXSCxVFmqpr1lid54tPBQI0DVr\n"\
            "YbhHo74rYh/uHFQV/gnYMFpjIxqsVJUeXF7UHHXT\n"\
            "-----END RSA PRIVATE KEY-----\n\0";
            std::string PRIMARYPrivateKey ="-----BEGIN RSA PRIVATE KEY-----\n"\
            "MIICXQIBAAKBgQCpLwVdn8aCfJ241DIn0/2YkdvhWz3hR+YuUIw+EOLlW3x9ZGAK\n"\
            "uqhaHDA7SOCNyj0WyDkFLJt0XAo0vyRMXLQLFeGC4z4ms6F1UX5jOLeaEKFCQrCz\n"\
            "nb6co2mI7AOrd3TOevimw3IMVC2uXyRRmUfI1zlzQJ3ZLXZfp3HEtsb+swIDAQAB\n"\
            "AoGAMsKP2pJIjffznnPUBI0fJjqE/S1R+JiI4FZ8bVbFrMjxJfIKus1cG4Ufl3n7\n"\
            "Ha+S+pXiPcIvdg7zZPYAXwTlY3zrq52zpT8DBjO1baUr1mwFCuUHOb0TSMtQQAOA\n"\
            "bb8ygA5ychZ26d0y+fO0Uyf+RvyinhYVQj/WD7RC+7bxzAECQQDeN1doCjOBUE6d\n"\
            "AYKS2/PWD4qzE/2oO/Pp/d3b2miljqkWCT3740Z8s/q3fOaA2G1bdqVyGJzSCvIc\n"\
            "MQYu7EoBAkEAwuenQ8AABmzXMv0nIIHcKCqyIYghU26PEX7/az0ta8I/k7G6w4I3\n"\
            "xdYSgVWX52ymSz4t3hmjqiTNxBYofw9AswJBAITXPI9bmdKyOWyunGpCxw+ultHb\n"\
            "H4ZU4Ml/FAP8Gxbto7Mir0VHZrnhtf2+0uUvfKBy2xDMGeOO25Fy1Slz5AECQGLN\n"\
            "2o5O2/Rl4qYXjWk+S6QVGN5Ey1gnOOHOtp7fkZIKE/7EX4BTGADzp8fm4y3Pd3G3\n"\
            "YLsx1jUfmSwpa3gV/F8CQQDFycTr9E6OcnKCBLhxsjrgZv1KZGiclX2zY7qVBb9P\n"\
            "FVu6D07msJ2urIrVXjQaK44EXQig/bEy0aQGU29+FbMd\n"\
            "-----END RSA PRIVATE KEY-----\n\0";
            std::string ESPGhostPublicKey = "-----BEGIN PUBLIC KEY-----\n"\
            "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC0IL4OdPyoRu4UGRJ1ozlR098q\n"\
            "nysZ58Kt9NdgcRARAXyo6DNbiurM5GSF/KC6iR70yB65IzUkkIsUqlsNoRw+SLbn\n"\
            "HITVlQx51BwX6C2OxnvuLUSf58jj2uS9EIlc75yTO3wNUn6JOsjkac9LICRg5OBt\n"\
            "U2Gm/YbwSSJSIVh9YwIDAQAB\n"\
            "-----END PUBLIC KEY-----\n\0";
            std::string ESPGhostPrivateKey = "-----BEGIN RSA PRIVATE KEY-----\n"\
            "MIICXQIBAAKBgQC0IL4OdPyoRu4UGRJ1ozlR098qnysZ58Kt9NdgcRARAXyo6DNb\n"\
            "iurM5GSF/KC6iR70yB65IzUkkIsUqlsNoRw+SLbnHITVlQx51BwX6C2OxnvuLUSf\n"\
            "58jj2uS9EIlc75yTO3wNUn6JOsjkac9LICRg5OBtU2Gm/YbwSSJSIVh9YwIDAQAB\n"\
            "AoGBAJOyGUSvZg/xAPwXxswQh0pyKJXBJ005fKa4kPUyH9Nh3b59jsbr9DUGvOok\n"\
            "SAJLtsdNY+u4U7XkkzXhrYFSbl27BJ7iUd4Cqxdj2SgGOWy5pE/g/CuZv9jhUv1O\n"\
            "pyAJja3sD5ifIcysQKCRuRtpddN9K+U8qCXG1PBO9gOdt4NBAkEA+/tRa7KCwyu5\n"\
            "roL1e1oKrVI7+UyeRCZU0/xh2z75UrKcvWTyV5JCHjIohib5mofwyrQKJSVK1+B/\n"\
            "opbnL93jgwJBALcAFzfEJjK86UFWBUl5w3kDYwK0YSM4hRV1FISJZgP4CIVB7Nti\n"\
            "NwJhhEwTPCcmhpoz2Ic3Agq9cI9+v2eNeKECQQCMYF/lnZcXX56bx702MN1N3Ugo\n"\
            "BDUPRkNvrWReneCL35FOOZSAkKxg4hSNV90p552Pb2lPdE0Toq+4RlUY54Y9AkAg\n"\
            "rmt3EhigRwyBFpOrI4u9ONWdzgjq+VULF/WqFb+aRdl/st5R76BTVIcD7wt3EnJH\n"\
            "Z6rruu8qa9qGReR44dMBAkBrUF+M6SD88nxhoWoa4panB3K0N4cHXoOsPkgE1R04\n"\
            "JImH91ZKvAsMdscq0Y8untye9jAiEhW7Q3bhfbD5d0bZ\n"\
            "-----END RSA PRIVATE KEY-----\n\0";
            std::string consumerPublicKey = "-----BEGIN PUBLIC KEY-----\n"\
            "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDDU1JmhVozvrgOZzP5SmLXRSWn\n"\
            "8Xm+GxvaKCpiHLz/ZGwKBpah9t43bmrD2dFjK4CAdJhuaU8SIIuHZ7+wvr2o8pRp\n"\
            "Uy5GkLPquKumYa6r3pB7D6bN4A5gGx1KfAAp5sWAMwtGQy5b/X3nhdi5FCGO1nqV\n"\
            "BH5ID0jWjf6qba0OZwIDAQAB\n"\
            "-----END PUBLIC KEY-----\n\0";
            std::string consumerPrivateKey = "-----BEGIN RSA PRIVATE KEY-----\n"\
            "MIICXQIBAAKBgQDDU1JmhVozvrgOZzP5SmLXRSWn8Xm+GxvaKCpiHLz/ZGwKBpah\n"\
            "9t43bmrD2dFjK4CAdJhuaU8SIIuHZ7+wvr2o8pRpUy5GkLPquKumYa6r3pB7D6bN\n"\
            "4A5gGx1KfAAp5sWAMwtGQy5b/X3nhdi5FCGO1nqVBH5ID0jWjf6qba0OZwIDAQAB\n"\
            "AoGBALaevpnv8CgVFs0QnL1lWQhgNBbTZLnjaqULjKTyrZoy7/hntIp138n/yH6Y\n"\
            "tnZ8A8cWPDZu62jNWOcej1DW5UbfP6SseL9kY/6CKdXzLhypOCwhWvJdigoksnr/\n"\
            "qX4T1O/afM2Zt6uVJXbhJUqYA7Av2xozOu7/NqRZhfSnmJYhAkEA8TAwX3eTONxQ\n"\
            "b9zd1aCm5G0etBkoguKEoYFlVh21l5CyyEPMExBpVzfA2EnF58j9oyUxJlyXYjYb\n"\
            "V5LzohEgvwJBAM9SG3A+hKD76eg+08o20Eu/Lzq1x9JaV7MC7t+o0yBxPfT3bFLF\n"\
            "oPm2NkroZqegvIC0Mi6caZVYCXtQX22GVFkCQQCsLly6KqEdXJrH4QM0+OVr+UKM\n"\
            "C9HR7mgE7F0tpzP2KcSYF15e1b5+Fz1Cqxfvqpr3lxuJaVxhLoNCd5QaROP9AkBs\n"\
            "Phk3VFzY1E3+xPa86Dt+oNYe2BP5HtQVEZ34RmPc12MayCfVW3u9hMX5/DbEpGZm\n"\
            "6C4fKejyYWGp8X+xZmYZAkAmiOWCjrzOdeAIxbXxDBu6PzSSRPKuvNGSaZVY5s1j\n"\
            "FskxaonJ4eBaZGee2IEkVuo4oLuFJRaBZ03mWEqR/8PD\n"\
            "-----END RSA PRIVATE KEY-----\n\0";
            std::string consumerOnePublicKey = "-----BEGIN PUBLIC KEY-----\n"\
            "MIGeMA0GCSqGSIb3DQEBAQUAA4GMADCBiAKBgHUiOU/DOpxSHJlJ7vIJJPOp+SwF\n"\
            "5G/+10vN/NaDlVgOxdi5LpIK711HnIpZvScoJVM26vfWrRq80GgBT13TVWe/OdMw\n"\
            "9kgjLr8Np2VhYi6QKX7f/TVpBaNSOL1WtDX2pVWn6CvExL1/McULNcV3W6DDDRBB\n"\
            "DSUH0PyaaF9UG2xvAgMBAAE=\n"\
            "-----END PUBLIC KEY-----\n\0";
            std::string consumerOnePrivateKey = "-----BEGIN RSA PRIVATE KEY-----\n"\
            "MIICXAIBAAKBgHUiOU/DOpxSHJlJ7vIJJPOp+SwF5G/+10vN/NaDlVgOxdi5LpIK\n"\
            "711HnIpZvScoJVM26vfWrRq80GgBT13TVWe/OdMw9kgjLr8Np2VhYi6QKX7f/TVp\n"\
            "BaNSOL1WtDX2pVWn6CvExL1/McULNcV3W6DDDRBBDSUH0PyaaF9UG2xvAgMBAAEC\n"\
            "gYBYSIIvtvKO7Wh2tH2u2OuL5zlRT+Ob8dRtYwTE3YoLQ58kuHX8RtegU6ANupNR\n"\
            "VvXC4Wp5f8kalOLy8MHT8ru0H7bQXhMQduUx2fZMnYMn3QA1GzTa8QdvN19adXaV\n"\
            "YdZ3FQfZSX0shkiiF72BDJdTH8tbIK0DVKgavue25ywi4QJBALedKpkwot9YvfVF\n"\
            "SXbPVt7DioLdBNoe8z3ZXgf3FnID+dcTvsoS+bLJMpHCREFniM7MZ2chSGBNA/jb\n"\
            "5O0uajECQQCjT6882CZ1tgCaIbIA3u2QLT1VjdLwNbJsVJ/NW3tNPQBFXQeqeATd\n"\
            "csuzMaDdwzYzJcBYJUFXMQ0KXzKh6PifAkBPfrYwuMPrWupzRFo4HDCKJbLUUYiG\n"\
            "PxwkzPVPEYUgO+RxX9Uce/arlzxTqv4OKxsqHYkvN1O0jeOIXPvSTGQhAkEAk/24\n"\
            "4l1J9O+aehJA2XMrOrUUWLwtxAoVBwBk09ar1OGLpk/Yi405c2f82gUGLxAgxJbY\n"\
            "R8hy/s1vcM+19W1GJwJBALVMUNse3valISwfjzTuRJu5lGw/M4Mqr3xHo9OMExCF\n"\
            "twJk43FM5t2TxkWw4+S1UF+6U10LDFac+msWVnYiMdk=\n"\
            "-----END RSA PRIVATE KEY-----\n\0";
            std::string consumerTwoPublicKey = "-----BEGIN PUBLIC KEY-----\n"\
            "MIGeMA0GCSqGSIb3DQEBAQUAA4GMADCBiAKBgHP2M0+wH/dUB4h/nxcREhqqlLLN\n"\
            "aFeyQlk8+fgTww/1KBgFMpoWldxHwPIaf9svUGn23WWPojbCiPg4epPD5xlG5AzS\n"\
            "cTk19eLyWkn48l6Kncaq+hK4fNl/OpGTsrkvsxJE1JPxkdnnFC0ftoS9l3d7XEeN\n"\
            "T/N98mLctoVRemoBAgMBAAE=\n"\
            "-----END PUBLIC KEY-----\n\0";
            std::string consumerTwoPrivateKey = "-----BEGIN RSA PRIVATE KEY-----\n"\
            "MIICXAIBAAKBgHP2M0+wH/dUB4h/nxcREhqqlLLNaFeyQlk8+fgTww/1KBgFMpoW\n"\
            "ldxHwPIaf9svUGn23WWPojbCiPg4epPD5xlG5AzScTk19eLyWkn48l6Kncaq+hK4\n"\
            "fNl/OpGTsrkvsxJE1JPxkdnnFC0ftoS9l3d7XEeNT/N98mLctoVRemoBAgMBAAEC\n"\
            "gYAqyFOn+8/fXRYmawdT6E1onzSbPyNW74KBD/kdaJUX5xfyvMAtj1Wc3rA14kb3\n"\
            "o94kKhsGpFZxo5YLFjNLHwS8kknFGg6o2KFUdOJDrXt8AWih9+PH9xdrhv8whYQ6\n"\
            "U9HuQ9SrOxwzu3beCDKNo6wdKXb48V1+h5Ltl043YxxoEQJBAM+wYXAuPLUtTnm2\n"\
            "aCJ/xwKEv447yG4OSwmGUj8I8YDkNRcGnJb1wCZBfBzinxo6uEZyxBBcuWXzExD+\n"\
            "hsjv2F0CQQCO75NqRLv9OAkn8ZRRNdWj5PFhezf5slJheVevf2uXLG/qmDOD/hSy\n"\
            "Iauhvi7x6kDWG8TlM9r5HmU7v8z4CC31AkEAlk6qVgvAd8bQUpnGh6E+H/5+u6tc\n"\
            "hPnnKH+DqocIcSW3nnB0h3RXmdguwKDqUItKrgGFD+pravMeS2BJn8alKQJADcMX\n"\
            "aHXxu7G3f/cMlr0eMseKTh51Bz+aGvR58tXL4CRRKj85Ro4iI/yF7+7FylkJoRPP\n"\
            "Y1oYzT+V86jF7WOsOQJBAJcOnGfuSeL6J6CnX8qwCLFLUgq0xNuBKrbY8yd9i55V\n"\
            "GCwNGqUERhT/fEQzTNS93M4hdO1ZPiTuHNsyDX9bB4M=\n"\
            "-----END RSA PRIVATE KEY-----\n\0";
            std::string consumerThreePublicKey = "-----BEGIN PUBLIC KEY-----\n"\
            "MIGeMA0GCSqGSIb3DQEBAQUAA4GMADCBiAKBgGYbWFWoj54OUnBeyGVR4Nl2z1sX\n"\
            "7c+89Djhjf//uvMicAKIOm2Qpxsvqjmu3HT72obON5aJnfdtuGeFBWKp6iAf34VW\n"\
            "bYFXyYi0EyAZ5/IRVKqdL1OmnE2HbL09MvqLN9mf0cQLF7F0zhA1BOK8v5cDsdhs\n"\
            "yAc75BQwK4W7fPV1AgMBAAE=\n"\
            "-----END PUBLIC KEY-----\n\0";
            std::string consumerThreePrivateKey = "-----BEGIN RSA PRIVATE KEY-----\n"\
            "MIICWgIBAAKBgGYbWFWoj54OUnBeyGVR4Nl2z1sX7c+89Djhjf//uvMicAKIOm2Q\n"\
            "pxsvqjmu3HT72obON5aJnfdtuGeFBWKp6iAf34VWbYFXyYi0EyAZ5/IRVKqdL1Om\n"\
            "nE2HbL09MvqLN9mf0cQLF7F0zhA1BOK8v5cDsdhsyAc75BQwK4W7fPV1AgMBAAEC\n"\
            "gYBDUQ3o00t0Ec8qBGMr4VJTLLBCR0Fi7xk4EwRyXWAcgXzFPpj+uRpiowcgJndz\n"\
            "rMOCkpw1ZB9ADBdCMIG/bXRLyhWNycLjS90v/xR8mc7l1Bp3GkvhDaE47liEou9Z\n"\
            "eUf7R4KHPlgajw1N82hN66b2lCKTOfKN0Erumtnvb2VpFQJBAKrdjJxmj+dck4lJ\n"\
            "M2+jpfqBfgjtBjK1eCdCjMHyLzyIQnoEWX0nNAiR/YPDjSvfZgC906irk8rHse6n\n"\
            "1rYUtCcCQQCY+2hQy7R5BRaC4lxleBPhBkOOvK4+8toYL4cjJ6Vs4fwfuv85dY2r\n"\
            "CkSKdP98/4h8tdiG9Hb63iqWEoJbVP8DAkA2dgFwhHxPuUFJW+yqI9Yun4GlpK76\n"\
            "EL9ofBzTYLdPqvrNW3+OlaVPKgFNM1JW7pDboaXKOE4WL0p2kEauqLC3AkBDG74l\n"\
            "+0Y+9GFspvnbNTKbOtE180UIriXICElivYiO3A9qp/A/93Ze+pEYn7xXzAzGkErc\n"\
            "4K602/KYhKQUhpObAkADNOrBLtq0UgzqmLVss1E9xiJVSfoNxH0gl2xvnOC7QIUC\n"\
            "oidaU2BH/dDtFoEFcqtsJPxuYbqWp0qeUht5E1oq\n"\
            "-----END RSA PRIVATE KEY-----\n\0";
    };
}

#endif