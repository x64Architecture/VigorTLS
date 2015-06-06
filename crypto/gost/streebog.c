/*
 * Copyright (c) 2014 Dmitry Eremin-Solenikov <dbaryshkov@gmail.com>
 * Copyright (c) 2005-2006 Cryptocom LTD
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 */

#include <machine/endian.h>

#include <stdlib.h>
#include <string.h>

#include <openssl/opensslconf.h>

#ifndef OPENSSL_NO_GOST
#include <openssl/crypto.h>
#include <openssl/objects.h>
#include <openssl/gost.h>

#include "gost_locl.h"

static const STREEBOG_LONG64 A_PI_table[8][256] = {
    { /* 0 */
      U64(0xd01f715b5c7ef8e6), U64(0x16fa240980778325), U64(0xa8a42e857ee049c8),
      U64(0x6ac1068fa186465b), U64(0x6e417bd7a2e9320b), U64(0x665c8167a437daab),
      U64(0x7666681aa89617f6), U64(0x4b959163700bdcf5), U64(0xf14be6b78df36248),
      U64(0xc585bd689a625cff), U64(0x9557d7fca67d82cb), U64(0x89f0b969af6dd366),
      U64(0xb0833d48749f6c35), U64(0xa1998c23b1ecbc7c), U64(0x8d70c431ac02a736),
      U64(0xd6dfbc2fd0a8b69e), U64(0x37aeb3e551fa198b), U64(0x0b7d128a40b5cf9c),
      U64(0x5a8f2008b5780cbc), U64(0xedec882284e333e5), U64(0xd25fc177d3c7c2ce),
      U64(0x5e0f5d50b61778ec), U64(0x1d873683c0c24cb9), U64(0xad040bcbb45d208c),
      U64(0x2f89a0285b853c76), U64(0x5732fff6791b8d58), U64(0x3e9311439ef6ec3f),
      U64(0xc9183a809fd3c00f), U64(0x83adf3f5260a01ee), U64(0xa6791941f4e8ef10),
      U64(0x103ae97d0ca1cd5d), U64(0x2ce948121dee1b4a), U64(0x39738421dbf2bf53),
      U64(0x093da2a6cf0cf5b4), U64(0xcd9847d89cbcb45f), U64(0xf9561c078b2d8ae8),
      U64(0x9c6a755a6971777f), U64(0xbc1ebaa0712ef0c5), U64(0x72e61542abf963a6),
      U64(0x78bb5fde229eb12e), U64(0x14ba94250fceb90d), U64(0x844d6697630e5282),
      U64(0x98ea08026a1e032f), U64(0xf06bbea144217f5c), U64(0xdb6263d11ccb377a),
      U64(0x641c314b2b8ee083), U64(0x320e96ab9b4770cf), U64(0x1ee7deb986a96b85),
      U64(0xe96cf57a878c47b5), U64(0xfdd6615f8842feb8), U64(0xc83862965601dd1b),
      U64(0x2ea9f83e92572162), U64(0xf876441142ff97fc), U64(0xeb2c455608357d9d),
      U64(0x5612a7e0b0c9904c), U64(0x6c01cbfb2d500823), U64(0x4548a6a7fa037a2d),
      U64(0xabc4c6bf388b6ef4), U64(0xbade77d4fdf8bebd), U64(0x799b07c8eb4cac3a),
      U64(0x0c9d87e805b19cf0), U64(0xcb588aac106afa27), U64(0xea0c1d40c1e76089),
      U64(0x2869354a1e816f1a), U64(0xff96d17307fbc490), U64(0x9f0a9d602f1a5043),
      U64(0x96373fc6e016a5f7), U64(0x5292dab8b3a6e41c), U64(0x9b8ae0382c752413),
      U64(0x4f15ec3b7364a8a5), U64(0x3fb349555724f12b), U64(0xc7c50d4415db66d7),
      U64(0x92b7429ee379d1a7), U64(0xd37f99611a15dfda), U64(0x231427c05e34a086),
      U64(0xa439a96d7b51d538), U64(0xb403401077f01865), U64(0xdda2aea5901d7902),
      U64(0x0a5d4a9c8967d288), U64(0xc265280adf660f93), U64(0x8bb0094520d4e94e),
      U64(0x2a29856691385532), U64(0x42a833c5bf072941), U64(0x73c64d54622b7eb2),
      U64(0x07e095624504536c), U64(0x8a905153e906f45a), U64(0x6f6123c16b3b2f1f),
      U64(0xc6e55552dc097bc3), U64(0x4468feb133d16739), U64(0xe211e7f0c7398829),
      U64(0xa2f96419f7879b40), U64(0x19074bdbc3ad38e9), U64(0xf4ebc3f9474e0b0c),
      U64(0x43886bd376d53455), U64(0xd8028beb5aa01046), U64(0x51f23282f5cdc320),
      U64(0xe7b1c2be0d84e16d), U64(0x081dfab006dee8a0), U64(0x3b33340d544b857b),
      U64(0x7f5bcabc679ae242), U64(0x0edd37c48a08a6d8), U64(0x81ed43d9a9b33bc6),
      U64(0xb1a3655ebd4d7121), U64(0x69a1eeb5e7ed6167), U64(0xf6ab73d5c8f73124),
      U64(0x1a67a3e185c61fd5), U64(0x2dc91004d43c065e), U64(0x0240b02c8fb93a28),
      U64(0x90f7f2b26cc0eb8f), U64(0x3cd3a16f114fd617), U64(0xaae49ea9f15973e0),
      U64(0x06c0cd748cd64e78), U64(0xda423bc7d5192a6e), U64(0xc345701c16b41287),
      U64(0x6d2193ede4821537), U64(0xfcf639494190e3ac), U64(0x7c3b228621f1c57e),
      U64(0xfb16ac2b0494b0c0), U64(0xbf7e529a3745d7f9), U64(0x6881b6a32e3f7c73),
      U64(0xca78d2bad9b8e733), U64(0xbbfe2fc2342aa3a9), U64(0x0dbddffecc6381e4),
      U64(0x70a6a56e2440598e), U64(0xe4d12a844befc651), U64(0x8c509c2765d0ba22),
      U64(0xee8c6018c28814d9), U64(0x17da7c1f49a59e31), U64(0x609c4c1328e194d3),
      U64(0xb3e3d57232f44b09), U64(0x91d7aaa4a512f69b), U64(0x0ffd6fd243dabbcc),
      U64(0x50d26a943c1fde34), U64(0x6be15e9968545b4f), U64(0x94778fea6faf9fdf),
      U64(0x2b09dd7058ea4826), U64(0x677cd9716de5c7bf), U64(0x49d5214fffb2e6dd),
      U64(0x0360e83a466b273c), U64(0x1fc786af4f7b7691), U64(0xa0b9d435783ea168),
      U64(0xd49f0c035f118cb6), U64(0x01205816c9d21d14), U64(0xac2453dd7d8f3d98),
      U64(0x545217cc3f70aa64), U64(0x26b4028e9489c9c2), U64(0xdec2469fd6765e3e),
      U64(0x04807d58036f7450), U64(0xe5f17292823ddb45), U64(0xf30b569b024a5860),
      U64(0x62dcfc3fa758aefb), U64(0xe84cad6c4e5e5aa1), U64(0xccb81fce556ea94b),
      U64(0x53b282ae7a74f908), U64(0x1b47fbf74c1402c1), U64(0x368eebf39828049f),
      U64(0x7afbeff2ad278b06), U64(0xbe5e0a8cfe97caed), U64(0xcfd8f7f413058e77),
      U64(0xf78b2bc301252c30), U64(0x4d555c17fcdd928d), U64(0x5f2f05467fc565f8),
      U64(0x24f4b2a21b30f3ea), U64(0x860dd6bbecb768aa), U64(0x4c750401350f8f99),
      U64(0x0000000000000000), U64(0xecccd0344d312ef1), U64(0xb5231806be220571),
      U64(0xc105c030990d28af), U64(0x653c695de25cfd97), U64(0x159acc33c61ca419),
      U64(0xb89ec7f872418495), U64(0xa9847693b73254dc), U64(0x58cf90243ac13694),
      U64(0x59efc832f3132b80), U64(0x5c4fed7c39ae42c4), U64(0x828dabe3efd81cfa),
      U64(0xd13f294d95ace5f2), U64(0x7d1b7a90e823d86a), U64(0xb643f03cf849224d),
      U64(0x3df3f979d89dcb03), U64(0x7426d836272f2dde), U64(0xdfe21e891fa4432a),
      U64(0x3a136c1b9d99986f), U64(0xfa36f43dcd46add4), U64(0xc025982650df35bb),
      U64(0x856d3e81aadc4f96), U64(0xc4a5e57e53b041eb), U64(0x4708168b75ba4005),
      U64(0xaf44bbe73be41aa4), U64(0x971767d029c4b8e3), U64(0xb9be9feebb939981),
      U64(0x215497ecd18d9aae), U64(0x316e7e91dd2c57f3), U64(0xcef8afe2dad79363),
      U64(0x3853dc371220a247), U64(0x35ee03c9de4323a3), U64(0xe6919aa8c456fc79),
      U64(0xe05157dc4880b201), U64(0x7bdbb7e464f59612), U64(0x127a59518318f775),
      U64(0x332ecebd52956ddb), U64(0x8f30741d23bb9d1e), U64(0xd922d3fd93720d52),
      U64(0x7746300c61440ae2), U64(0x25d4eab4d2e2eefe), U64(0x75068020eefd30ca),
      U64(0x135a01474acaea61), U64(0x304e268714fe4ae7), U64(0xa519f17bb283c82c),
      U64(0xdc82f6b359cf6416), U64(0x5baf781e7caa11a8), U64(0xb2c38d64fb26561d),
      U64(0x34ce5bdf17913eb7), U64(0x5d6fb56af07c5fd0), U64(0x182713cd0a7f25fd),
      U64(0x9e2ac576e6c84d57), U64(0x9aaab82ee5a73907), U64(0xa3d93c0f3e558654),
      U64(0x7e7b92aaae48ff56), U64(0x872d8ead256575be), U64(0x41c8dbfff96c0e7d),
      U64(0x99ca5014a3cc1e3b), U64(0x40e883e930be1369), U64(0x1ca76e95091051ad),
      U64(0x4e35b42dbab6b5b1), U64(0x05a0254ecabd6944), U64(0xe1710fca8152af15),
      U64(0xf22b0e8dcb984574), U64(0xb763a82a319b3f59), U64(0x63fca4296e8ab3ef),
      U64(0x9d4a2d4ca0a36a6b), U64(0xe331bfe60eeb953d), U64(0xd5bf541596c391a2),
      U64(0xf5cb9bef8e9c1618), U64(0x46284e9dbc685d11), U64(0x2074cffa185f87ba),
      U64(0xbd3ee2b6b8fcedd1), U64(0xae64e3f1f23607b0), U64(0xfeb68965ce29d984),
      U64(0x55724fdaf6a2b770), U64(0x29496d5cd753720e), U64(0xa75941573d3af204),
      U64(0x8e102c0bea69800a), U64(0x111ab16bc573d049), U64(0xd7ffe439197aab8a),
      U64(0xefac380e0b5a09cd), U64(0x48f579593660fbc9), U64(0x22347fd697e6bd92),
      U64(0x61bc1405e13389c7), U64(0x4ab5c975b9d9c1e1), U64(0x80cd1bcf606126d2),
      U64(0x7186fd78ed92449a), U64(0x93971a882aabccb3), U64(0x88d0e17f66bfce72),
      U64(0x27945a985d5bd4d6) },
    { /* 1 */
      U64(0xde553f8c05a811c8), U64(0x1906b59631b4f565), U64(0x436e70d6b1964ff7),
      U64(0x36d343cb8b1e9d85), U64(0x843dfacc858aab5a), U64(0xfdfc95c299bfc7f9),
      U64(0x0f634bdea1d51fa2), U64(0x6d458b3b76efb3cd), U64(0x85c3f77cf8593f80),
      U64(0x3c91315fbe737cb2), U64(0x2148b03366ace398), U64(0x18f8b8264c6761bf),
      U64(0xc830c1c495c9fb0f), U64(0x981a76102086a0aa), U64(0xaa16012142f35760),
      U64(0x35cc54060c763cf6), U64(0x42907d66cc45db2d), U64(0x8203d44b965af4bc),
      U64(0x3d6f3cefc3a0e868), U64(0xbc73ff69d292bda7), U64(0x8722ed0102e20a29),
      U64(0x8f8185e8cd34deb7), U64(0x9b0561dda7ee01d9), U64(0x5335a0193227fad6),
      U64(0xc9cecc74e81a6fd5), U64(0x54f5832e5c2431ea), U64(0x99e47ba05d553470),
      U64(0xf7bee756acd226ce), U64(0x384e05a5571816fd), U64(0xd1367452a47d0e6a),
      U64(0xf29fde1c386ad85b), U64(0x320c77316275f7ca), U64(0xd0c879e2d9ae9ab0),
      U64(0xdb7406c69110ef5d), U64(0x45505e51a2461011), U64(0xfc029872e46c5323),
      U64(0xfa3cb6f5f7bc0cc5), U64(0x031f17cd8768a173), U64(0xbd8df2d9af41297d),
      U64(0x9d3b4f5ab43e5e3f), U64(0x4071671b36feee84), U64(0x716207e7d3e3b83d),
      U64(0x48d20ff2f9283a1a), U64(0x27769eb4757cbc7e), U64(0x5c56ebc793f2e574),
      U64(0xa48b474f9ef5dc18), U64(0x52cbada94ff46e0c), U64(0x60c7da982d8199c6),
      U64(0x0e9d466edc068b78), U64(0x4eec2175eaf865fc), U64(0x550b8e9e21f7a530),
      U64(0x6b7ba5bc653fec2b), U64(0x5eb7f1ba6949d0dd), U64(0x57ea94e3db4c9099),
      U64(0xf640eae6d101b214), U64(0xdd4a284182c0b0bb), U64(0xff1d8fbf6304f250),
      U64(0xb8accb933bf9d7e8), U64(0xe8867c478eb68c4d), U64(0x3f8e2692391bddc1),
      U64(0xcb2fd60912a15a7c), U64(0xaec935dbab983d2f), U64(0xf55ffd2b56691367),
      U64(0x80e2ce366ce1c115), U64(0x179bf3f8edb27e1d), U64(0x01fe0db07dd394da),
      U64(0xda8a0b76ecc37b87), U64(0x44ae53e1df9584cb), U64(0xb310b4b77347a205),
      U64(0xdfab323c787b8512), U64(0x3b511268d070b78e), U64(0x65e6e3d2b9396753),
      U64(0x6864b271e2574d58), U64(0x259784c98fc789d7), U64(0x02e11a7dfabb35a9),
      U64(0x8841a6dfa337158b), U64(0x7ade78c39b5dcdd0), U64(0xb7cf804d9a2cc84a),
      U64(0x20b6bd831b7f7742), U64(0x75bd331d3a88d272), U64(0x418f6aab4b2d7a5e),
      U64(0xd9951cbb6babdaf4), U64(0xb6318dfde7ff5c90), U64(0x1f389b112264aa83),
      U64(0x492c024284fbaec0), U64(0xe33a0363c608f9a0), U64(0x2688930408af28a4),
      U64(0xc7538a1a341ce4ad), U64(0x5da8e677ee2171ae), U64(0x8c9e92254a5c7fc4),
      U64(0x63d8cd55aae938b5), U64(0x29ebd8daa97a3706), U64(0x959827b37be88aa1),
      U64(0x1484e4356adadf6e), U64(0xa7945082199d7d6b), U64(0xbf6ce8a455fa1cd4),
      U64(0x9cc542eac9edcae5), U64(0x79c16f0e1c356ca3), U64(0x89bfab6fdee48151),
      U64(0xd4174d1830c5f0ff), U64(0x9258048415eb419d), U64(0x6139d72850520d1c),
      U64(0x6a85a80c18ec78f1), U64(0xcd11f88e0171059a), U64(0xcceff53e7ca29140),
      U64(0xd229639f2315af19), U64(0x90b91ef9ef507434), U64(0x5977d28d074a1be1),
      U64(0x311360fce51d56b9), U64(0xc093a92d5a1f2f91), U64(0x1a19a25bb6dc5416),
      U64(0xeb996b8a09de2d3e), U64(0xfee3820f1ed7668a), U64(0xd7085ad5b7ad518c),
      U64(0x7fff41890fe53345), U64(0xec5948bd67dde602), U64(0x2fd5f65dbaaa68e0),
      U64(0xa5754affe32648c2), U64(0xf8ddac880d07396c), U64(0x6fa491468c548664),
      U64(0x0c7c5c1326bdbed1), U64(0x4a33158f03930fb3), U64(0x699abfc19f84d982),
      U64(0xe4fa2054a80b329c), U64(0x6707f9af438252fa), U64(0x08a368e9cfd6d49e),
      U64(0x47b1442c58fd25b8), U64(0xbbb3dc5ebc91769b), U64(0x1665fe489061eac7),
      U64(0x33f27a811fa66310), U64(0x93a609346838d547), U64(0x30ed6d4c98cec263),
      U64(0x1dd9816cd8df9f2a), U64(0x94662a03063b1e7b), U64(0x83fdd9fbeb896066),
      U64(0x7b207573e68e590a), U64(0x5f49fc0a149a4407), U64(0x343259b671a5a82c),
      U64(0xfbc2bb458a6f981f), U64(0xc272b350a0a41a38), U64(0x3aaf1fd8ada32354),
      U64(0x6cbb868b0b3c2717), U64(0xa2b569c88d2583fe), U64(0xf180c9d1bf027928),
      U64(0xaf37386bd64ba9f5), U64(0x12bacab2790a8088), U64(0x4c0d3b0810435055),
      U64(0xb2eeb9070e9436df), U64(0xc5b29067cea7d104), U64(0xdcb425f1ff132461),
      U64(0x4f122cc5972bf126), U64(0xac282fa651230886), U64(0xe7e537992f6393ef),
      U64(0xe61b3a2952b00735), U64(0x709c0a57ae302ce7), U64(0xe02514ae416058d3),
      U64(0xc44c9dd7b37445de), U64(0x5a68c5408022ba92), U64(0x1c278cdca50c0bf0),
      U64(0x6e5a9cf6f18712be), U64(0x86dce0b17f319ef3), U64(0x2d34ec2040115d49),
      U64(0x4bcd183f7e409b69), U64(0x2815d56ad4a9a3dc), U64(0x24698979f2141d0d),
      U64(0x0000000000000000), U64(0x1ec696a15fb73e59), U64(0xd86b110b16784e2e),
      U64(0x8e7f8858b0e74a6d), U64(0x063e2e8713d05fe6), U64(0xe2c40ed3bbdb6d7a),
      U64(0xb1f1aeca89fc97ac), U64(0xe1db191e3cb3cc09), U64(0x6418ee62c4eaf389),
      U64(0xc6ad87aa49cf7077), U64(0xd6f65765ca7ec556), U64(0x9afb6c6dda3d9503),
      U64(0x7ce05644888d9236), U64(0x8d609f95378feb1e), U64(0x23a9aa4e9c17d631),
      U64(0x6226c0e5d73aac6f), U64(0x56149953a69f0443), U64(0xeeb852c09d66d3ab),
      U64(0x2b0ac2a753c102af), U64(0x07c023376e03cb3c), U64(0x2ccae1903dc2c993),
      U64(0xd3d76e2f5ec63bc3), U64(0x9e2458973356ff4c), U64(0xa66a5d32644ee9b1),
      U64(0x0a427294356de137), U64(0x783f62be61e6f879), U64(0x1344c70204d91452),
      U64(0x5b96c8f0fdf12e48), U64(0xa90916ecc59bf613), U64(0xbe92e5142829880e),
      U64(0x727d102a548b194e), U64(0x1be7afebcb0fc0cc), U64(0x3e702b2244c8491b),
      U64(0xd5e940a84d166425), U64(0x66f9f41f3e51c620), U64(0xabe80c913f20c3ba),
      U64(0xf07ec461c2d1edf2), U64(0xf361d3ac45b94c81), U64(0x0521394a94b8fe95),
      U64(0xadd622162cf09c5c), U64(0xe97871f7f3651897), U64(0xf4a1f09b2bba87bd),
      U64(0x095d6559b2054044), U64(0x0bbc7f2448be75ed), U64(0x2af4cf172e129675),
      U64(0x157ae98517094bb4), U64(0x9fda55274e856b96), U64(0x914713499283e0ee),
      U64(0xb952c623462a4332), U64(0x74433ead475b46a8), U64(0x8b5eb112245fb4f8),
      U64(0xa34b6478f0f61724), U64(0x11a5dd7ffe6221fb), U64(0xc16da49d27ccbb4b),
      U64(0x76a224d0bde07301), U64(0x8aa0bca2598c2022), U64(0x4df336b86d90c48f),
      U64(0xea67663a740db9e4), U64(0xef465f70e0b54771), U64(0x39b008152acb8227),
      U64(0x7d1e5bf4f55e06ec), U64(0x105bd0cf83b1b521), U64(0x775c2960c033e7db),
      U64(0x7e014c397236a79f), U64(0x811cc386113255cf), U64(0xeda7450d1a0e72d8),
      U64(0x5889df3d7a998f3b), U64(0x2e2bfbedc779fc3a), U64(0xce0eef438619a4e9),
      U64(0x372d4e7bf6cd095f), U64(0x04df34fae96b6a4f), U64(0xf923a13870d4adb6),
      U64(0xa1aa7e050a4d228d), U64(0xa8f71b5cb84862c9), U64(0xb52e9a306097fde3),
      U64(0x0d8251a35b6e2a0b), U64(0x2257a7fee1c442eb), U64(0x73831d9a29588d94),
      U64(0x51d4ba64c89ccf7f), U64(0x502ab7d4b54f5ba5), U64(0x97793dce8153bf08),
      U64(0xe5042de4d5d8a646), U64(0x9687307efc802bd2), U64(0xa05473b5779eb657),
      U64(0xb4d097801d446939), U64(0xcff0e2f3fbca3033), U64(0xc38cbee0dd778ee2),
      U64(0x464f499c252eb162), U64(0xcad1dbb96f72cea6), U64(0xba4dd1eec142e241),
      U64(0xb00fa37af42f0376) },
    { /* 2 */
      U64(0xcce4cd3aa968b245), U64(0x089d5484e80b7faf), U64(0x638246c1b3548304),
      U64(0xd2fe0ec8c2355492), U64(0xa7fbdf7ff2374eee), U64(0x4df1600c92337a16),
      U64(0x84e503ea523b12fb), U64(0x0790bbfd53ab0c4a), U64(0x198a780f38f6ea9d),
      U64(0x2ab30c8f55ec48cb), U64(0xe0f7fed6b2c49db5), U64(0xb6ecf3f422cadbdc),
      U64(0x409c9a541358df11), U64(0xd3ce8a56dfde3fe3), U64(0xc3e9224312c8c1a0),
      U64(0x0d6dfa58816ba507), U64(0xddf3e1b179952777), U64(0x04c02a42748bb1d9),
      U64(0x94c2abff9f2decb8), U64(0x4f91752da8f8acf4), U64(0x78682befb169bf7b),
      U64(0xe1c77a48af2ff6c4), U64(0x0c5d7ec69c80ce76), U64(0x4cc1e4928fd81167),
      U64(0xfeed3d24d9997b62), U64(0x518bb6dfc3a54a23), U64(0x6dbf2d26151f9b90),
      U64(0xb5bc624b05ea664f), U64(0xe86aaa525acfe21a), U64(0x4801ced0fb53a0be),
      U64(0xc91463e6c00868ed), U64(0x1027a815cd16fe43), U64(0xf67069a0319204cd),
      U64(0xb04ccc976c8abce7), U64(0xc0b9b3fc35e87c33), U64(0xf380c77c58f2de65),
      U64(0x50bb3241de4e2152), U64(0xdf93f490435ef195), U64(0xf1e0d25d62390887),
      U64(0xaf668bfb1a3c3141), U64(0xbc11b251f00a7291), U64(0x73a5eed47e427d47),
      U64(0x25bee3f6ee4c3b2e), U64(0x43cc0beb34786282), U64(0xc824e778dde3039c),
      U64(0xf97d86d98a327728), U64(0xf2b043e24519b514), U64(0xe297ebf7880f4b57),
      U64(0x3a94a49a98fab688), U64(0x868516cb68f0c419), U64(0xeffa11af0964ee50),
      U64(0xa4ab4ec0d517f37d), U64(0xa9c6b498547c567a), U64(0x8e18424f80fbbbb6),
      U64(0x0bcdc53bcf2bc23c), U64(0x137739aaea3643d0), U64(0x2c1333ec1bac2ff0),
      U64(0x8d48d3f0a7db0625), U64(0x1e1ac3f26b5de6d7), U64(0xf520f81f16b2b95e),
      U64(0x9f0f6ec450062e84), U64(0x0130849e1deb6b71), U64(0xd45e31ab8c7533a9),
      U64(0x652279a2fd14e43f), U64(0x3209f01e70f1c927), U64(0xbe71a770cac1a473),
      U64(0x0e3d6be7a64b1894), U64(0x7ec8148cff29d840), U64(0xcb7476c7fac3be0f),
      U64(0x72956a4a63a91636), U64(0x37f95ec21991138f), U64(0x9e3fea5a4ded45f5),
      U64(0x7b38ba50964902e8), U64(0x222e580bbde73764), U64(0x61e253e0899f55e6),
      U64(0xfc8d2805e352ad80), U64(0x35994be3235ac56d), U64(0x09add01af5e014de),
      U64(0x5e8659a6780539c6), U64(0xb17c48097161d796), U64(0x026015213acbd6e2),
      U64(0xd1ae9f77e515e901), U64(0xb7dc776a3f21b0ad), U64(0xaba6a1b96eb78098),
      U64(0x9bcf4486248d9f5d), U64(0x582666c536455efd), U64(0xfdbdac9bfeb9c6f1),
      U64(0xc47999be4163cdea), U64(0x765540081722a7ef), U64(0x3e548ed8ec710751),
      U64(0x3d041f67cb51bac2), U64(0x7958af71ac82d40a), U64(0x36c9da5c047a78fe),
      U64(0xed9a048e33af38b2), U64(0x26ee7249c96c86bd), U64(0x900281bdeba65d61),
      U64(0x11172c8bd0fd9532), U64(0xea0abf73600434f8), U64(0x42fc8f75299309f3),
      U64(0x34a9cf7d3eb1ae1c), U64(0x2b838811480723ba), U64(0x5ce64c8742ceef24),
      U64(0x1adae9b01fd6570e), U64(0x3c349bf9d6bad1b3), U64(0x82453c891c7b75c0),
      U64(0x97923a40b80d512b), U64(0x4a61dbf1c198765c), U64(0xb48ce6d518010d3e),
      U64(0xcfb45c858e480fd6), U64(0xd933cbf30d1e96ae), U64(0xd70ea014ab558e3a),
      U64(0xc189376228031742), U64(0x9262949cd16d8b83), U64(0xeb3a3bed7def5f89),
      U64(0x49314a4ee6b8cbcf), U64(0xdcc3652f647e4c06), U64(0xda635a4c2a3e2b3d),
      U64(0x470c21a940f3d35b), U64(0x315961a157d174b4), U64(0x6672e81dda3459ac),
      U64(0x5b76f77a1165e36e), U64(0x445cb01667d36ec8), U64(0xc5491d205c88a69b),
      U64(0x456c34887a3805b9), U64(0xffddb9bac4721013), U64(0x99af51a71e4649bf),
      U64(0xa15be01cbc7729d5), U64(0x52db2760e485f7b0), U64(0x8c78576eba306d54),
      U64(0xae560f6507d75a30), U64(0x95f22f6182c687c9), U64(0x71c5fbf54489aba5),
      U64(0xca44f259e728d57e), U64(0x88b87d2ccebbdc8d), U64(0xbab18d32be4a15aa),
      U64(0x8be8ec93e99b611e), U64(0x17b713e89ebdf209), U64(0xb31c5d284baa0174),
      U64(0xeeca9531148f8521), U64(0xb8d198138481c348), U64(0x8988f9b2d350b7fc),
      U64(0xb9e11c8d996aa839), U64(0x5a4673e40c8e881f), U64(0x1687977683569978),
      U64(0xbf4123eed72acf02), U64(0x4ea1f1b3b513c785), U64(0xe767452be16f91ff),
      U64(0x7505d1b730021a7c), U64(0xa59bca5ec8fc980c), U64(0xad069eda20f7e7a3),
      U64(0x38f4b1bba231606a), U64(0x60d2d77e94743e97), U64(0x9affc0183966f42c),
      U64(0x248e6768f3a7505f), U64(0xcdd449a4b483d934), U64(0x87b59255751baf68),
      U64(0x1bea6d2e023d3c7f), U64(0x6b1f12455b5ffcab), U64(0x743555292de9710d),
      U64(0xd8034f6d10f5fddf), U64(0xc6198c9f7ba81b08), U64(0xbb8109aca3a17edb),
      U64(0xfa2d1766ad12cabb), U64(0xc729080166437079), U64(0x9c5fff7b77269317),
      U64(0x0000000000000000), U64(0x15d706c9a47624eb), U64(0x6fdf38072fd44d72),
      U64(0x5fb6dd3865ee52b7), U64(0xa33bf53d86bcff37), U64(0xe657c1b5fc84fa8e),
      U64(0xaa962527735cebe9), U64(0x39c43525bfda0b1b), U64(0x204e4d2a872ce186),
      U64(0x7a083ece8ba26999), U64(0x554b9c9db72efbfa), U64(0xb22cd9b656416a05),
      U64(0x96a2bedea5e63a5a), U64(0x802529a826b0a322), U64(0x8115ad363b5bc853),
      U64(0x8375b81701901eb1), U64(0x3069e53f4a3a1fc5), U64(0xbd2136cfede119e0),
      U64(0x18bafc91251d81ec), U64(0x1d4a524d4c7d5b44), U64(0x05f0aedc6960daa8),
      U64(0x29e39d3072ccf558), U64(0x70f57f6b5962c0d4), U64(0x989fd53903ad22ce),
      U64(0xf84d024797d91c59), U64(0x547b1803aac5908b), U64(0xf0d056c37fd263f6),
      U64(0xd56eb535919e58d8), U64(0x1c7ad6d351963035), U64(0x2e7326cd2167f912),
      U64(0xac361a443d1c8cd2), U64(0x697f076461942a49), U64(0x4b515f6fdc731d2d),
      U64(0x8ad8680df4700a6f), U64(0x41ac1eca0eb3b460), U64(0x7d988533d80965d3),
      U64(0xa8f6300649973d0b), U64(0x7765c4960ac9cc9e), U64(0x7ca801adc5e20ea2),
      U64(0xdea3700e5eb59ae4), U64(0xa06b6482a19c42a4), U64(0x6a2f96db46b497da),
      U64(0x27def6d7d487edcc), U64(0x463ca5375d18b82a), U64(0xa6cb5be1efdc259f),
      U64(0x53eba3fef96e9cc1), U64(0xce84d81b93a364a7), U64(0xf4107c810b59d22f),
      U64(0x333974806d1aa256), U64(0x0f0def79bba073e5), U64(0x231edc95a00c5c15),
      U64(0xe437d494c64f2c6c), U64(0x91320523f64d3610), U64(0x67426c83c7df32dd),
      U64(0x6eefbc99323f2603), U64(0x9d6f7be56acdf866), U64(0x5916e25b2bae358c),
      U64(0x7ff89012e2c2b331), U64(0x035091bf2720bd93), U64(0x561b0d22900e4669),
      U64(0x28d319ae6f279e29), U64(0x2f43a2533c8c9263), U64(0xd09e1be9f8fe8270),
      U64(0xf740ed3e2c796fbc), U64(0xdb53ded237d5404c), U64(0x62b2c25faebfe875),
      U64(0x0afd41a5d2c0a94d), U64(0x6412fd3ce0ff8f4e), U64(0xe3a76f6995e42026),
      U64(0x6c8fa9b808f4f0e1), U64(0xc2d9a6dd0f23aad1), U64(0x8f28c6d19d10d0c7),
      U64(0x85d587744fd0798a), U64(0xa20b71a39b579446), U64(0x684f83fa7c7f4138),
      U64(0xe507500adba4471d), U64(0x3f640a46f19a6c20), U64(0x1247bd34f7dd28a1),
      U64(0x2d23b77206474481), U64(0x93521002cc86e0f2), U64(0x572b89bc8de52d18),
      U64(0xfb1d93f8b0f9a1ca), U64(0xe95a2ecc4724896b), U64(0x3ba420048511ddf9),
      U64(0xd63e248ab6bee54b), U64(0x5dd6c8195f258455), U64(0x06a03f634e40673b),
      U64(0x1f2a476c76b68da6), U64(0x217ec9b49ac78af7), U64(0xecaa80102e4453c3),
      U64(0x14e78257b99d4f9a) },
    { /* 3 */
      U64(0x20329b2cc87bba05), U64(0x4f5eb6f86546a531), U64(0xd4f44775f751b6b1),
      U64(0x8266a47b850dfa8b), U64(0xbb986aa15a6ca985), U64(0xc979eb08f9ae0f99),
      U64(0x2da6f447a2375ea1), U64(0x1e74275dcd7d8576), U64(0xbc20180a800bc5f8),
      U64(0xb4a2f701b2dc65be), U64(0xe726946f981b6d66), U64(0x48e6c453bf21c94c),
      U64(0x42cad9930f0a4195), U64(0xefa47b64aacccd20), U64(0x71180a8960409a42),
      U64(0x8bb3329bf6a44e0c), U64(0xd34c35de2d36dacc), U64(0xa92f5b7cbc23dc96),
      U64(0xb31a85aa68bb09c3), U64(0x13e04836a73161d2), U64(0xb24dfc4129c51d02),
      U64(0x8ae44b70b7da5acd), U64(0xe671ed84d96579a7), U64(0xa4bb3417d66f3832),
      U64(0x4572ab38d56d2de8), U64(0xb1b47761ea47215c), U64(0xe81c09cf70aba15d),
      U64(0xffbdb872ce7f90ac), U64(0xa8782297fd5dc857), U64(0x0d946f6b6a4ce4a4),
      U64(0xe4df1f4f5b995138), U64(0x9ebc71edca8c5762), U64(0x0a2c1dc0b02b88d9),
      U64(0x3b503c115d9d7b91), U64(0xc64376a8111ec3a2), U64(0xcec199a323c963e4),
      U64(0xdc76a87ec58616f7), U64(0x09d596e073a9b487), U64(0x14583a9d7d560daf),
      U64(0xf4c6dc593f2a0cb4), U64(0xdd21d19584f80236), U64(0x4a4836983ddde1d3),
      U64(0xe58866a41ae745f9), U64(0xf591a5b27e541875), U64(0x891dc05074586693),
      U64(0x5b068c651810a89e), U64(0xa30346bc0c08544f), U64(0x3dbf3751c684032d),
      U64(0x2a1e86ec785032dc), U64(0xf73f5779fca830ea), U64(0xb60c05ca30204d21),
      U64(0x0cc316802b32f065), U64(0x8770241bdd96be69), U64(0xb861e18199ee95db),
      U64(0xf805cad91418fcd1), U64(0x29e70dccbbd20e82), U64(0xc7140f435060d763),
      U64(0x0f3a9da0e8b0cc3b), U64(0xa2543f574d76408e), U64(0xbd7761e1c175d139),
      U64(0x4b1f4f737ca3f512), U64(0x6dc2df1f2fc137ab), U64(0xf1d05c3967b14856),
      U64(0xa742bf3715ed046c), U64(0x654030141d1697ed), U64(0x07b872abda676c7d),
      U64(0x3ce84eba87fa17ec), U64(0xc1fb0403cb79afdf), U64(0x3e46bc7105063f73),
      U64(0x278ae987121cd678), U64(0xa1adb4778ef47cd0), U64(0x26dd906c5362c2b9),
      U64(0x05168060589b44e2), U64(0xfbfc41f9d79ac08f), U64(0x0e6de44ba9ced8fa),
      U64(0x9feb08068bf243a3), U64(0x7b341749d06b129b), U64(0x229c69e74a87929a),
      U64(0xe09ee6c4427c011b), U64(0x5692e30e725c4c3a), U64(0xda99a33e5e9f6e4b),
      U64(0x353dd85af453a36b), U64(0x25241b4c90e0fee7), U64(0x5de987258309d022),
      U64(0xe230140fc0802984), U64(0x93281e86a0c0b3c6), U64(0xf229d719a4337408),
      U64(0x6f6c2dd4ad3d1f34), U64(0x8ea5b2fbae3f0aee), U64(0x8331dd90c473ee4a),
      U64(0x346aa1b1b52db7aa), U64(0xdf8f235e06042aa9), U64(0xcc6f6b68a1354b7b),
      U64(0x6c95a6f46ebf236a), U64(0x52d31a856bb91c19), U64(0x1a35ded6d498d555),
      U64(0xf37eaef2e54d60c9), U64(0x72e181a9a3c2a61c), U64(0x98537aad51952fde),
      U64(0x16f6c856ffaa2530), U64(0xd960281e9d1d5215), U64(0x3a0745fa1ce36f50),
      U64(0x0b7b642bf1559c18), U64(0x59a87eae9aec8001), U64(0x5e100c05408bec7c),
      U64(0x0441f98b19e55023), U64(0xd70dcc5534d38aef), U64(0x927f676de1bea707),
      U64(0x9769e70db925e3e5), U64(0x7a636ea29115065a), U64(0x468b201816ef11b6),
      U64(0xab81a9b73edff409), U64(0xc0ac7de88a07bb1e), U64(0x1f235eb68c0391b7),
      U64(0x6056b074458dd30f), U64(0xbe8eeac102f7ed67), U64(0xcd381283e04b5fba),
      U64(0x5cbefecec277c4e3), U64(0xd21b4c356c48ce0d), U64(0x1019c31664b35d8c),
      U64(0x247362a7d19eea26), U64(0xebe582efb3299d03), U64(0x02aef2cb82fc289f),
      U64(0x86275df09ce8aaa8), U64(0x28b07427faac1a43), U64(0x38a9b7319e1f47cf),
      U64(0xc82e92e3b8d01b58), U64(0x06ef0b409b1978bc), U64(0x62f842bfc771fb90),
      U64(0x9904034610eb3b1f), U64(0xded85ab5477a3e68), U64(0x90d195a663428f98),
      U64(0x5384636e2ac708d8), U64(0xcbd719c37b522706), U64(0xae9729d76644b0eb),
      U64(0x7c8c65e20a0c7ee6), U64(0x80c856b007f1d214), U64(0x8c0b40302cc32271),
      U64(0xdbcedad51fe17a8a), U64(0x740e8ae938dbdea0), U64(0xa615c6dc549310ad),
      U64(0x19cc55f6171ae90b), U64(0x49b1bdb8fe5fdd8d), U64(0xed0a89af2830e5bf),
      U64(0x6a7aadb4f5a65bd6), U64(0x7e22972988f05679), U64(0xf952b3325566e810),
      U64(0x39fecedadf61530e), U64(0x6101c99f04f3c7ce), U64(0x2e5f7f6761b562ff),
      U64(0xf08725d226cf5c97), U64(0x63af3b54860fef51), U64(0x8ff2cb10ef411e2f),
      U64(0x884ab9bb35267252), U64(0x4df04433e7ba8dae), U64(0x9afd8866d3690741),
      U64(0x66b9bb34de94abb3), U64(0x9baaf18d92171380), U64(0x543c11c5f0a064a5),
      U64(0x17a1b1bdbed431f1), U64(0xb5f58eeaf3a2717f), U64(0xc355f6c849858740),
      U64(0xec5df044694ef17e), U64(0xd83751f5dc6346d4), U64(0xfc4433520dfdacf2),
      U64(0x0000000000000000), U64(0x5a51f58e596ebc5f), U64(0x3285aaf12e34cf16),
      U64(0x8d5c39db6dbd36b0), U64(0x12b731dde64f7513), U64(0x94906c2d7aa7dfbb),
      U64(0x302b583aacc8e789), U64(0x9d45facd090e6b3c), U64(0x2165e2c78905aec4),
      U64(0x68d45f7f775a7349), U64(0x189b2c1d5664fdca), U64(0xe1c99f2f030215da),
      U64(0x6983269436246788), U64(0x8489af3b1e148237), U64(0xe94b702431d5b59c),
      U64(0x33d2d31a6f4adbd7), U64(0xbfd9932a4389f9a6), U64(0xb0e30e8aab39359d),
      U64(0xd1e2c715afcaf253), U64(0x150f43763c28196e), U64(0xc4ed846393e2eb3d),
      U64(0x03f98b20c3823c5e), U64(0xfd134ab94c83b833), U64(0x556b682eb1de7064),
      U64(0x36c4537a37d19f35), U64(0x7559f30279a5ca61), U64(0x799ae58252973a04),
      U64(0x9c12832648707ffd), U64(0x78cd9c6913e92ec5), U64(0x1d8dac7d0effb928),
      U64(0x439da0784e745554), U64(0x413352b3cc887dcb), U64(0xbacf134a1b12bd44),
      U64(0x114ebafd25cd494d), U64(0x2f08068c20cb763e), U64(0x76a07822ba27f63f),
      U64(0xeab2fb04f25789c2), U64(0xe3676de481fe3d45), U64(0x1b62a73d95e6c194),
      U64(0x641749ff5c68832c), U64(0xa5ec4dfc97112cf3), U64(0xf6682e92bdd6242b),
      U64(0x3f11c59a44782bb2), U64(0x317c21d1edb6f348), U64(0xd65ab5be75ad9e2e),
      U64(0x6b2dd45fb4d84f17), U64(0xfaab381296e4d44e), U64(0xd0b5befeeeb4e692),
      U64(0x0882ef0b32d7a046), U64(0x512a91a5a83b2047), U64(0x963e9ee6f85bf724),
      U64(0x4e09cf132438b1f0), U64(0x77f701c9fb59e2fe), U64(0x7ddb1c094b726a27),
      U64(0x5f4775ee01f5f8bd), U64(0x9186ec4d223c9b59), U64(0xfeeac1998f01846d),
      U64(0xac39db1ce4b89874), U64(0xb75b7c21715e59e0), U64(0xafc0503c273aa42a),
      U64(0x6e3b543fec430bf5), U64(0x704f7362213e8e83), U64(0x58ff0745db9294c0),
      U64(0x67eec2df9feabf72), U64(0xa0facd9ccf8a6811), U64(0xb936986ad890811a),
      U64(0x95c715c63bd9cb7a), U64(0xca8060283a2c33c7), U64(0x507de84ee9453486),
      U64(0x85ded6d05f6a96f6), U64(0x1cdad5964f81ade9), U64(0xd5a33e9eb62fa270),
      U64(0x40642b588df6690a), U64(0x7f75eec2c98e42b8), U64(0x2cf18dace3494a60),
      U64(0x23cb100c0bf9865b), U64(0xeef3028febb2d9e1), U64(0x4425d2d394133929),
      U64(0xaad6d05c7fa1e0c8), U64(0xad6ea2f7a5c68cb5), U64(0xc2028f2308fb9381),
      U64(0x819f2f5b468fc6d5), U64(0xc5bafd88d29cfffc), U64(0x47dc59f357910577),
      U64(0x2b49ff07392e261d), U64(0x57c59ae5332258fb), U64(0x73b6f842e2bcb2dd),
      U64(0xcf96e04862b77725), U64(0x4ca73dd8a6c4996f), U64(0x015779eb417e14c1),
      U64(0x37932a9176af8bf4) },
    { /* 4 */
      U64(0x190a2c9b249df23e), U64(0x2f62f8b62263e1e9), U64(0x7a7f754740993655),
      U64(0x330b7ba4d5564d9f), U64(0x4c17a16a46672582), U64(0xb22f08eb7d05f5b8),
      U64(0x535f47f40bc148cc), U64(0x3aec5d27d4883037), U64(0x10ed0a1825438f96),
      U64(0x516101f72c233d17), U64(0x13cc6f949fd04eae), U64(0x739853c441474bfd),
      U64(0x653793d90d3f5b1b), U64(0x5240647b96b0fc2f), U64(0x0c84890ad27623e0),
      U64(0xd7189b32703aaea3), U64(0x2685de3523bd9c41), U64(0x99317c5b11bffefa),
      U64(0x0d9baa854f079703), U64(0x70b93648fbd48ac5), U64(0xa80441fce30bc6be),
      U64(0x7287704bdc36ff1e), U64(0xb65384ed33dc1f13), U64(0xd36417343ee34408),
      U64(0x39cd38ab6e1bf10f), U64(0x5ab861770a1f3564), U64(0x0ebacf09f594563b),
      U64(0xd04572b884708530), U64(0x3cae9722bdb3af47), U64(0x4a556b6f2f5cbaf2),
      U64(0xe1704f1f76c4bd74), U64(0x5ec4ed7144c6dfcf), U64(0x16afc01d4c7810e6),
      U64(0x283f113cd629ca7a), U64(0xaf59a8761741ed2d), U64(0xeed5a3991e215fac),
      U64(0x3bf37ea849f984d4), U64(0xe413e096a56ce33c), U64(0x2c439d3a98f020d1),
      U64(0x637559dc6404c46b), U64(0x9e6c95d1e5f5d569), U64(0x24bb9836045fe99a),
      U64(0x44efa466dac8ecc9), U64(0xc6eab2a5c80895d6), U64(0x803b50c035220cc4),
      U64(0x0321658cba93c138), U64(0x8f9ebc465dc7ee1c), U64(0xd15a5137190131d3),
      U64(0x0fa5ec8668e5e2d8), U64(0x91c979578d1037b1), U64(0x0642ca05693b9f70),
      U64(0xefca80168350eb4f), U64(0x38d21b24f36a45ec), U64(0xbeab81e1af73d658),
      U64(0x8cbfd9cae7542f24), U64(0xfd19cc0d81f11102), U64(0x0ac6430fbb4dbc90),
      U64(0x1d76a09d6a441895), U64(0x2a01573ff1cbbfa1), U64(0xb572e161894fde2b),
      U64(0x8124734fa853b827), U64(0x614b1fdf43e6b1b0), U64(0x68ac395c4238cc18),
      U64(0x21d837bfd7f7b7d2), U64(0x20c714304a860331), U64(0x5cfaab726324aa14),
      U64(0x74c5ba4eb50d606e), U64(0xf3a3030474654739), U64(0x23e671bcf015c209),
      U64(0x45f087e947b9582a), U64(0xd8bd77b418df4c7b), U64(0xe06f6c90ebb50997),
      U64(0x0bd96080263c0873), U64(0x7e03f9410e40dcfe), U64(0xb8e94be4c6484928),
      U64(0xfb5b0608e8ca8e72), U64(0x1a2b49179e0e3306), U64(0x4e29e76961855059),
      U64(0x4f36c4e6fcf4e4ba), U64(0x49740ee395cf7bca), U64(0xc2963ea386d17f7d),
      U64(0x90d65ad810618352), U64(0x12d34c1b02a1fa4d), U64(0xfa44258775bb3a91),
      U64(0x18150f14b9ec46dd), U64(0x1491861e6b9a653d), U64(0x9a1019d7ab2c3fc2),
      U64(0x3668d42d06fe13d7), U64(0xdcc1fbb25606a6d0), U64(0x969490dd795a1c22),
      U64(0x3549b1a1bc6dd2ef), U64(0xc94f5e23a0ed770e), U64(0xb9f6686b5b39fdcb),
      U64(0xc4d4f4a6efeae00d), U64(0xe732851a1fff2204), U64(0x94aad6de5eb869f9),
      U64(0x3f8ff2ae07206e7f), U64(0xfe38a9813b62d03a), U64(0xa7a1ad7a8bee2466),
      U64(0x7b6056c8dde882b6), U64(0x302a1e286fc58ca7), U64(0x8da0fa457a259bc7),
      U64(0xb3302b64e074415b), U64(0x5402ae7eff8b635f), U64(0x08f8050c9cafc94b),
      U64(0xae468bf98a3059ce), U64(0x88c355cca98dc58f), U64(0xb10e6d67c7963480),
      U64(0xbad70de7e1aa3cf3), U64(0xbfb4a26e320262bb), U64(0xcb711820870f02d5),
      U64(0xce12b7a954a75c9d), U64(0x563ce87dd8691684), U64(0x9f73b65e7884618a),
      U64(0x2b1e74b06cba0b42), U64(0x47cec1ea605b2df1), U64(0x1c698312f735ac76),
      U64(0x5fdbcefed9b76b2c), U64(0x831a354c8fb1cdfc), U64(0x820516c312c0791f),
      U64(0xb74ca762aeadabf0), U64(0xfc06ef821c80a5e1), U64(0x5723cbf24518a267),
      U64(0x9d4df05d5f661451), U64(0x588627742dfd40bf), U64(0xda8331b73f3d39a0),
      U64(0x17b0e392d109a405), U64(0xf965400bcf28fba9), U64(0x7c3dbf4229a2a925),
      U64(0x023e460327e275db), U64(0x6cd0b55a0ce126b3), U64(0xe62da695828e96e7),
      U64(0x42ad6e63b3f373b9), U64(0xe50cc319381d57df), U64(0xc5cbd729729b54ee),
      U64(0x46d1e265fd2a9912), U64(0x6428b056904eeff8), U64(0x8be23040131e04b7),
      U64(0x6709d5da2add2ec0), U64(0x075de98af44a2b93), U64(0x8447dcc67bfbe66f),
      U64(0x6616f655b7ac9a23), U64(0xd607b8bded4b1a40), U64(0x0563af89d3a85e48),
      U64(0x3db1b4ad20c21ba4), U64(0x11f22997b8323b75), U64(0x292032b34b587e99),
      U64(0x7f1cdace9331681d), U64(0x8e819fc9c0b65aff), U64(0xa1e3677fe2d5bb16),
      U64(0xcd33d225ee349da5), U64(0xd9a2543b85aef898), U64(0x795e10cbfa0af76d),
      U64(0x25a4bbb9992e5d79), U64(0x78413344677b438e), U64(0xf0826688cef68601),
      U64(0xd27b34bba392f0eb), U64(0x551d8df162fad7bc), U64(0x1e57c511d0d7d9ad),
      U64(0xdeffbdb171e4d30b), U64(0xf4feea8e802f6caa), U64(0xa480c8f6317de55e),
      U64(0xa0fc44f07fa40ff5), U64(0x95b5f551c3c9dd1a), U64(0x22f952336d6476ea),
      U64(0x0000000000000000), U64(0xa6be8ef5169f9085), U64(0xcc2cf1aa73452946),
      U64(0x2e7ddb39bf12550a), U64(0xd526dd3157d8db78), U64(0x486b2d6c08becf29),
      U64(0x9b0f3a58365d8b21), U64(0xac78cdfaadd22c15), U64(0xbc95c7e28891a383),
      U64(0x6a927f5f65dab9c3), U64(0xc3891d2c1ba0cb9e), U64(0xeaa92f9f50f8b507),
      U64(0xcf0d9426c9d6e87e), U64(0xca6e3baf1a7eb636), U64(0xab25247059980786),
      U64(0x69b31ad3df4978fb), U64(0xe2512a93cc577c4c), U64(0xff278a0ea61364d9),
      U64(0x71a615c766a53e26), U64(0x89dc764334fc716c), U64(0xf87a638452594f4a),
      U64(0xf2bc208be914f3da), U64(0x8766b94ac1682757), U64(0xbbc82e687cdb8810),
      U64(0x626a7a53f9757088), U64(0xa2c202f358467a2e), U64(0x4d0882e5db169161),
      U64(0x09e7268301de7da8), U64(0xe897699c771ac0dc), U64(0xc8507dac3d9cc3ed),
      U64(0xc0a878a0a1330aa6), U64(0x978bb352e42ba8c1), U64(0xe9884a13ea6b743f),
      U64(0x279afdbabecc28a2), U64(0x047c8c064ed9eaab), U64(0x507e2278b15289f4),
      U64(0x599904fbb08cf45c), U64(0xbd8ae46d15e01760), U64(0x31353da7f2b43844),
      U64(0x8558ff49e68a528c), U64(0x76fbfc4d92ef15b5), U64(0x3456922e211c660c),
      U64(0x86799ac55c1993b4), U64(0x3e90d1219a51da9c), U64(0x2d5cbeb505819432),
      U64(0x982e5fd48cce4a19), U64(0xdb9c1238a24c8d43), U64(0xd439febecaa96f9b),
      U64(0x418c0bef0960b281), U64(0x158ea591f6ebd1de), U64(0x1f48e69e4da66d4e),
      U64(0x8afd13cf8e6fb054), U64(0xf5e1c9011d5ed849), U64(0xe34e091c5126c8af),
      U64(0xad67ee7530a398f6), U64(0x43b24dec2e82c75a), U64(0x75da99c1287cd48d),
      U64(0x92e81cdb3783f689), U64(0xa3dd217cc537cecd), U64(0x60543c50de970553),
      U64(0x93f73f54aaf2426a), U64(0xa91b62737e7a725d), U64(0xf19d4507538732e2),
      U64(0x77e4dfc20f9ea156), U64(0x7d229ccdb4d31dc6), U64(0x1b346a98037f87e5),
      U64(0xedf4c615a4b29e94), U64(0x4093286094110662), U64(0xb0114ee85ae78063),
      U64(0x6ff1d0d6b672e78b), U64(0x6dcf96d591909250), U64(0xdfe09e3eec9567e8),
      U64(0x3214582b4827f97c), U64(0xb46dc2ee143e6ac8), U64(0xf6c0ac8da7cd1971),
      U64(0xebb60c10cd8901e4), U64(0xf7df8f023abcad92), U64(0x9c52d3d2c217a0b2),
      U64(0x6b8d5cd0f8ab0d20), U64(0x3777f7a29b8fa734), U64(0x011f238f9d71b4e3),
      U64(0xc1b75b2f3c42be45), U64(0x5de588fdfe551ef7), U64(0x6eeef3592b035368),
      U64(0xaa3a07ffc4e9b365), U64(0xecebe59a39c32a77), U64(0x5ba742f8976e8187),
      U64(0x4b4a48e0b22d0e11), U64(0xddded83dcb771233), U64(0xa59feb79ac0c51bd),
      U64(0xc7f5912a55792135) },
    { /* 5 */
      U64(0x6d6ae04668a9b08a), U64(0x3ab3f04b0be8c743), U64(0xe51e166b54b3c908),
      U64(0xbe90a9eb35c2f139), U64(0xb2c7066637f2bec1), U64(0xaa6945613392202c),
      U64(0x9a28c36f3b5201eb), U64(0xddce5a93ab536994), U64(0x0e34133ef6382827),
      U64(0x52a02ba1ec55048b), U64(0xa2f88f97c4b2a177), U64(0x8640e513ca2251a5),
      U64(0xcdf1d36258137622), U64(0xfe6cb708dedf8ddb), U64(0x8a174a9ec8121e5d),
      U64(0x679896036b81560e), U64(0x59ed033395795fee), U64(0x1dd778ab8b74edaf),
      U64(0xee533ef92d9f926d), U64(0x2a8c79baf8a8d8f5), U64(0x6bcf398e69b119f6),
      U64(0xe20491742fafdd95), U64(0x276488e0809c2aec), U64(0xea955b82d88f5cce),
      U64(0x7102c63a99d9e0c4), U64(0xf9763017a5c39946), U64(0x429fa2501f151b3d),
      U64(0x4659c72bea05d59e), U64(0x984b7fdccf5a6634), U64(0xf742232953fbb161),
      U64(0x3041860e08c021c7), U64(0x747bfd9616cd9386), U64(0x4bb1367192312787),
      U64(0x1b72a1638a6c44d3), U64(0x4a0e68a6e8359a66), U64(0x169a5039f258b6ca),
      U64(0xb98a2ef44edee5a4), U64(0xd9083fe85e43a737), U64(0x967f6ce239624e13),
      U64(0x8874f62d3c1a7982), U64(0x3c1629830af06e3f), U64(0x9165ebfd427e5a8e),
      U64(0xb5dd81794ceeaa5c), U64(0x0de8f15a7834f219), U64(0x70bd98ede3dd5d25),
      U64(0xaccc9ca9328a8950), U64(0x56664eda1945ca28), U64(0x221db34c0f8859ae),
      U64(0x26dbd637fa98970d), U64(0x1acdffb4f068f932), U64(0x4585254f64090fa0),
      U64(0x72de245e17d53afa), U64(0x1546b25d7c546cf4), U64(0x207e0ffffb803e71),
      U64(0xfaaad2732bcf4378), U64(0xb462dfae36ea17bd), U64(0xcf926fd1ac1b11fd),
      U64(0xe0672dc7dba7ba4a), U64(0xd3fa49ad5d6b41b3), U64(0x8ba81449b216a3bc),
      U64(0x14f9ec8a0650d115), U64(0x40fc1ee3eb1d7ce2), U64(0x23a2ed9b758ce44f),
      U64(0x782c521b14fddc7e), U64(0x1c68267cf170504e), U64(0xbcf31558c1ca96e6),
      U64(0xa781b43b4ba6d235), U64(0xf6fd7dfe29ff0c80), U64(0xb0a4bad5c3fad91e),
      U64(0xd199f51ea963266c), U64(0x414340349119c103), U64(0x5405f269ed4dadf7),
      U64(0xabd61bb649969dcd), U64(0x6813dbeae7bdc3c8), U64(0x65fb2ab09f8931d1),
      U64(0xf1e7fae152e3181d), U64(0xc1a67cef5a2339da), U64(0x7a4feea8e0f5bba1),
      U64(0x1e0b9acf05783791), U64(0x5b8ebf8061713831), U64(0x80e53cdbcb3af8d9),
      U64(0x7e898bd315e57502), U64(0xc6bcfbf0213f2d47), U64(0x95a38e86b76e942d),
      U64(0x092e94218d243cba), U64(0x8339debf453622e7), U64(0xb11be402b9fe64ff),
      U64(0x57d9100d634177c9), U64(0xcc4e8db52217cbc3), U64(0x3b0cae9c71ec7aa2),
      U64(0xfb158ca451cbfe99), U64(0x2b33276d82ac6514), U64(0x01bf5ed77a04bde1),
      U64(0xc5601994af33f779), U64(0x75c4a3416cc92e67), U64(0xf3844652a6eb7fc2),
      U64(0x3487e375fdd0ef64), U64(0x18ae430704609eed), U64(0x4d14efb993298efb),
      U64(0x815a620cb13e4538), U64(0x125c354207487869), U64(0x9eeea614ce42cf48),
      U64(0xce2d3106d61fac1c), U64(0xbbe99247bad6827b), U64(0x071a871f7b1c149d),
      U64(0x2e4a1cc10db81656), U64(0x77a71ff298c149b8), U64(0x06a5d9c80118a97c),
      U64(0xad73c27e488e34b1), U64(0x443a7b981e0db241), U64(0xe3bbcfa355ab6074),
      U64(0x0af276450328e684), U64(0x73617a896dd1871b), U64(0x58525de4ef7de20f),
      U64(0xb7be3dcab8e6cd83), U64(0x19111dd07e64230c), U64(0x842359a03e2a367a),
      U64(0x103f89f1f3401fb6), U64(0xdc710444d157d475), U64(0xb835702334da5845),
      U64(0x4320fc876511a6dc), U64(0xd026abc9d3679b8d), U64(0x17250eee885c0b2b),
      U64(0x90dab52a387ae76f), U64(0x31fed8d972c49c26), U64(0x89cba8fa461ec463),
      U64(0x2ff5421677bcabb7), U64(0x396f122f85e41d7d), U64(0xa09b332430bac6a8),
      U64(0xc888e8ced7070560), U64(0xaeaf201ac682ee8f), U64(0x1180d7268944a257),
      U64(0xf058a43628e7a5fc), U64(0xbd4c4b8fbbce2b07), U64(0xa1246df34abe7b49),
      U64(0x7d5569b79be9af3c), U64(0xa9b5a705bd9efa12), U64(0xdb6b835baa4bc0e8),
      U64(0x05793bac8f147342), U64(0x21c1512881848390), U64(0xfdb0556c50d357e5),
      U64(0x613d4fcb6a99ff72), U64(0x03dce2648e0cda3e), U64(0xe949b9e6568386f0),
      U64(0xfc0f0bbb2ad7ea04), U64(0x6a70675913b5a417), U64(0x7f36d5046fe1c8e3),
      U64(0x0c57af8d02304ff8), U64(0x32223abdfcc84618), U64(0x0891caf6f720815b),
      U64(0xa63eeaec31a26fd4), U64(0x2507345374944d33), U64(0x49d28ac266394058),
      U64(0xf5219f9aa7f3d6be), U64(0x2d96fea583b4cc68), U64(0x5a31e1571b7585d0),
      U64(0x8ed12fe53d02d0fe), U64(0xdfade6205f5b0e4b), U64(0x4cabb16ee92d331a),
      U64(0x04c6657bf510cea3), U64(0xd73c2cd6a87b8f10), U64(0xe1d87310a1a307ab),
      U64(0x6cd5be9112ad0d6b), U64(0x97c032354366f3f2), U64(0xd4e0ceb22677552e),
      U64(0x0000000000000000), U64(0x29509bde76a402cb), U64(0xc27a9e8bd42fe3e4),
      U64(0x5ef7842cee654b73), U64(0xaf107ecdbc86536e), U64(0x3fcacbe784fcb401),
      U64(0xd55f90655c73e8cf), U64(0xe6c2f40fdabf1336), U64(0xe8f6e7312c873b11),
      U64(0xeb2a0555a28be12f), U64(0xe4a148bc2eb774e9), U64(0x9b979db84156bc0a),
      U64(0x6eb60222e6a56ab4), U64(0x87ffbbc4b026ec44), U64(0xc703a5275b3b90a6),
      U64(0x47e699fc9001687f), U64(0x9c8d1aa73a4aa897), U64(0x7cea3760e1ed12dd),
      U64(0x4ec80ddd1d2554c5), U64(0x13e36b957d4cc588), U64(0x5d2b66486069914d),
      U64(0x92b90999cc7280b0), U64(0x517cc9c56259deb5), U64(0xc937b619ad03b881),
      U64(0xec30824ad997f5b2), U64(0xa45d565fc5aa080b), U64(0xd6837201d27f32f1),
      U64(0x635ef3789e9198ad), U64(0x531f75769651b96a), U64(0x4f77530a6721e924),
      U64(0x486dd4151c3dfdb9), U64(0x5f48dafb9461f692), U64(0x375b011173dc355a),
      U64(0x3da9775470f4d3de), U64(0x8d0dcd81b30e0ac0), U64(0x36e45fc609d888bb),
      U64(0x55baacbe97491016), U64(0x8cb29356c90ab721), U64(0x76184125e2c5f459),
      U64(0x99f4210bb55edbd5), U64(0x6f095cf59ca1d755), U64(0x9f51f8c3b44672a9),
      U64(0x3538bda287d45285), U64(0x50c39712185d6354), U64(0xf23b1885dcefc223),
      U64(0x79930ccc6ef9619f), U64(0xed8fdc9da3934853), U64(0xcb540aaa590bdf5e),
      U64(0x5c94389f1a6d2cac), U64(0xe77daad8a0bbaed7), U64(0x28efc5090ca0bf2a),
      U64(0xbf2ff73c4fc64cd8), U64(0xb37858b14df60320), U64(0xf8c96ec0dfc724a7),
      U64(0x828680683f329f06), U64(0x941cd051cd6a29cc), U64(0xc3c5c05cae2b5e05),
      U64(0xb601631dc2e27062), U64(0xc01922382027843b), U64(0x24b86a840e90f0d2),
      U64(0xd245177a276ffc52), U64(0x0f8b4de98c3c95c6), U64(0x3e759530fef809e0),
      U64(0x0b4d2892792c5b65), U64(0xc4df4743d5374a98), U64(0xa5e20888bfaeb5ea),
      U64(0xba56cc90c0d23f9a), U64(0x38d04cf8ffe0a09c), U64(0x62e1adafe495254c),
      U64(0x0263bcb3f40867df), U64(0xcaeb547d230f62bf), U64(0x6082111c109d4293),
      U64(0xdad4dd8cd04f7d09), U64(0xefec602e579b2f8c), U64(0x1fb4c4187f7c8a70),
      U64(0xffd3e9dfa4db303a), U64(0x7bf0b07f9af10640), U64(0xf49ec14dddf76b5f),
      U64(0x8f6e713247066d1f), U64(0x339d646a86ccfbf9), U64(0x64447467e58d8c30),
      U64(0x2c29a072f9b07189), U64(0xd8b7613f24471ad6), U64(0x6627c8d41185ebef),
      U64(0xa347d140beb61c96), U64(0xde12b8f7255fb3aa), U64(0x9d324470404e1576),
      U64(0x9306574eb6763d51), U64(0xa80af9d2c79a47f3), U64(0x859c0777442e8b9b),
      U64(0x69ac853d9db97e29) },
    { /* 6 */
      U64(0xc3407dfc2de6377e), U64(0x5b9e93eea4256f77), U64(0xadb58fdd50c845e0),
      U64(0x5219ff11a75bed86), U64(0x356b61cfd90b1de9), U64(0xfb8f406e25abe037),
      U64(0x7a5a0231c0f60796), U64(0x9d3cd216e1f5020b), U64(0x0c6550fb6b48d8f3),
      U64(0xf57508c427ff1c62), U64(0x4ad35ffa71cb407d), U64(0x6290a2da1666aa6d),
      U64(0xe284ec2349355f9f), U64(0xb3c307c53d7c84ec), U64(0x05e23c0468365a02),
      U64(0x190bac4d6c9ebfa8), U64(0x94bbbee9e28b80fa), U64(0xa34fc777529cb9b5),
      U64(0xcc7b39f095bcd978), U64(0x2426addb0ce532e3), U64(0x7e79329312ce4fc7),
      U64(0xab09a72eebec2917), U64(0xf8d15499f6b9d6c2), U64(0x1a55b8babf8c895d),
      U64(0xdb8add17fb769a85), U64(0xb57f2f368658e81b), U64(0x8acd36f18f3f41f6),
      U64(0x5ce3b7bba50f11d3), U64(0x114dcc14d5ee2f0a), U64(0xb91a7fcded1030e8),
      U64(0x81d5425fe55de7a1), U64(0xb6213bc1554adeee), U64(0x80144ef95f53f5f2),
      U64(0x1e7688186db4c10c), U64(0x3b912965db5fe1bc), U64(0xc281715a97e8252d),
      U64(0x54a5d7e21c7f8171), U64(0x4b12535ccbc5522e), U64(0x1d289cefbea6f7f9),
      U64(0x6ef5f2217d2e729e), U64(0xe6a7dc819b0d17ce), U64(0x1b94b41c05829b0e),
      U64(0x33d7493c622f711e), U64(0xdcf7f942fa5ce421), U64(0x600fba8b7f7a8ecb),
      U64(0x46b60f011a83988e), U64(0x235b898e0dcf4c47), U64(0x957ab24f588592a9),
      U64(0x4354330572b5c28c), U64(0xa5f3ef84e9b8d542), U64(0x8c711e02341b2d01),
      U64(0x0b1874ae6a62a657), U64(0x1213d8e306fc19ff), U64(0xfe6d7c6a4d9dba35),
      U64(0x65ed868f174cd4c9), U64(0x88522ea0e6236550), U64(0x899322065c2d7703),
      U64(0xc01e690bfef4018b), U64(0x915982ed8abddaf8), U64(0xbe675b98ec3a4e4c),
      U64(0xa996bf7f82f00db1), U64(0xe1daf8d49a27696a), U64(0x2effd5d3dc8986e7),
      U64(0xd153a51f2b1a2e81), U64(0x18caa0ebd690adfb), U64(0x390e3134b243c51a),
      U64(0x2778b92cdff70416), U64(0x029f1851691c24a6), U64(0x5e7cafeacc133575),
      U64(0xfa4e4cc89fa5f264), U64(0x5a5f9f481e2b7d24), U64(0x484c47ab18d764db),
      U64(0x400a27f2a1a7f479), U64(0xaeeb9b2a83da7315), U64(0x721c626879869734),
      U64(0x042330a2d2384851), U64(0x85f672fd3765aff0), U64(0xba446b3a3e02061d),
      U64(0x73dd6ecec3888567), U64(0xffac70ccf793a866), U64(0xdfa9edb5294ed2d4),
      U64(0x6c6aea7014325638), U64(0x834a5a0e8c41c307), U64(0xcdba35562fb2cb2b),
      U64(0x0ad97808d06cb404), U64(0x0f3b440cb85aee06), U64(0xe5f9c876481f213b),
      U64(0x98deee1289c35809), U64(0x59018bbfcd394bd1), U64(0xe01bf47220297b39),
      U64(0xde68e1139340c087), U64(0x9fa3ca4788e926ad), U64(0xbb85679c840c144e),
      U64(0x53d8f3b71d55ffd5), U64(0x0da45c5dd146caa0), U64(0x6f34fe87c72060cd),
      U64(0x57fbc315cf6db784), U64(0xcee421a1fca0fdde), U64(0x3d2d0196607b8d4b),
      U64(0x642c8a29ad42c69a), U64(0x14aff010bdd87508), U64(0xac74837beac657b3),
      U64(0x3216459ad821634d), U64(0x3fb219c70967a9ed), U64(0x06bc28f3bb246cf7),
      U64(0xf2082c9126d562c6), U64(0x66b39278c45ee23c), U64(0xbd394f6f3f2878b9),
      U64(0xfd33689d9e8f8cc0), U64(0x37f4799eb017394f), U64(0x108cc0b26fe03d59),
      U64(0xda4bd1b1417888d6), U64(0xb09d1332ee6eb219), U64(0x2f3ed975668794b4),
      U64(0x58c0871977375982), U64(0x7561463d78ace990), U64(0x09876cff037e82f1),
      U64(0x7fb83e35a8c05d94), U64(0x26b9b58a65f91645), U64(0xef20b07e9873953f),
      U64(0x3148516d0b3355b8), U64(0x41cb2b541ba9e62a), U64(0x790416c613e43163),
      U64(0xa011d380818e8f40), U64(0x3a5025c36151f3ef), U64(0xd57095bdf92266d0),
      U64(0x498d4b0da2d97688), U64(0x8b0c3a57353153a5), U64(0x21c491df64d368e1),
      U64(0x8f2f0af5e7091bf4), U64(0x2da1c1240f9bb012), U64(0xc43d59a92ccc49da),
      U64(0xbfa6573e56345c1f), U64(0x828b56a8364fd154), U64(0x9a41f643e0df7caf),
      U64(0xbcf843c985266aea), U64(0x2b1de9d7b4bfdce5), U64(0x20059d79dedd7ab2),
      U64(0x6dabe6d6ae3c446b), U64(0x45e81bf6c991ae7b), U64(0x6351ae7cac68b83e),
      U64(0xa432e32253b6c711), U64(0xd092a9b991143cd2), U64(0xcac711032e98b58f),
      U64(0xd8d4c9e02864ac70), U64(0xc5fc550f96c25b89), U64(0xd7ef8dec903e4276),
      U64(0x67729ede7e50f06f), U64(0xeac28c7af045cf3d), U64(0xb15c1f945460a04a),
      U64(0x9cfddeb05bfb1058), U64(0x93c69abce3a1fe5e), U64(0xeb0380dc4a4bdd6e),
      U64(0xd20db1e8f8081874), U64(0x229a8528b7c15e14), U64(0x44291750739fbc28),
      U64(0xd3ccbd4e42060a27), U64(0xf62b1c33f4ed2a97), U64(0x86a8660ae4779905),
      U64(0xd62e814a2a305025), U64(0x477703a7a08d8add), U64(0x7b9b0e977af815c5),
      U64(0x78c51a60a9ea2330), U64(0xa6adfb733aaae3b7), U64(0x97e5aa1e3199b60f),
      U64(0x0000000000000000), U64(0xf4b404629df10e31), U64(0x5564db44a6719322),
      U64(0x9207961a59afec0d), U64(0x9624a6b88b97a45c), U64(0x363575380a192b1c),
      U64(0x2c60cd82b595a241), U64(0x7d272664c1dc7932), U64(0x7142769faa94a1c1),
      U64(0xa1d0df263b809d13), U64(0x1630e841d4c451ae), U64(0xc1df65ad44fa13d8),
      U64(0x13d2d445bcf20bac), U64(0xd915c546926abe23), U64(0x38cf3d92084dd749),
      U64(0xe766d0272103059d), U64(0xc7634d5effde7f2f), U64(0x077d2455012a7ea4),
      U64(0xedbfa82ff16fb199), U64(0xaf2a978c39d46146), U64(0x42953fa3c8bbd0df),
      U64(0xcb061da59496a7dc), U64(0x25e7a17db6eb20b0), U64(0x34aa6d6963050fba),
      U64(0xa76cf7d580a4f1e4), U64(0xf7ea10954ee338c4), U64(0xfcf2643b24819e93),
      U64(0xcf252d0746aeef8d), U64(0x4ef06f58a3f3082c), U64(0x563acfb37563a5d7),
      U64(0x5086e740ce47c920), U64(0x2982f186dda3f843), U64(0x87696aac5e798b56),
      U64(0x5d22bb1d1f010380), U64(0x035e14f7d31236f5), U64(0x3cec0d30da759f18),
      U64(0xf3c920379cdb7095), U64(0xb8db736b571e22bb), U64(0xdd36f5e44052f672),
      U64(0xaac8ab8851e23b44), U64(0xa857b3d938fe1fe2), U64(0x17f1e4e76eca43fd),
      U64(0xec7ea4894b61a3ca), U64(0x9e62c6e132e734fe), U64(0xd4b1991b432c7483),
      U64(0x6ad6c283af163acf), U64(0x1ce9904904a8e5aa), U64(0x5fbda34c761d2726),
      U64(0xf910583f4cb7c491), U64(0xc6a241f845d06d7c), U64(0x4f3163fe19fd1a7f),
      U64(0xe99c988d2357f9c8), U64(0x8eee06535d0709a7), U64(0x0efa48aa0254fc55),
      U64(0xb4be23903c56fa48), U64(0x763f52caabbedf65), U64(0xeee1bcd8227d876c),
      U64(0xe345e085f33b4dcc), U64(0x3e731561b369bbbe), U64(0x2843fd2067adea10),
      U64(0x2adce5710eb1ceb6), U64(0xb7e03767ef44ccbd), U64(0x8db012a48e153f52),
      U64(0x61ceb62dc5749c98), U64(0xe85d942b9959eb9b), U64(0x4c6f7709caef2c8a),
      U64(0x84377e5b8d6bbda3), U64(0x30895dcbb13d47eb), U64(0x74a04a9bc2a2fbc3),
      U64(0x6b17ce251518289c), U64(0xe438c4d0f2113368), U64(0x1fb784bed7bad35f),
      U64(0x9b80fae55ad16efc), U64(0x77fe5e6c11b0cd36), U64(0xc858095247849129),
      U64(0x08466059b97090a2), U64(0x01c10ca6ba0e1253), U64(0x6988d6747c040c3a),
      U64(0x6849dad2c60a1e69), U64(0x5147ebe67449db73), U64(0xc99905f4fd8a837a),
      U64(0x991fe2b433cd4a5a), U64(0xf09734c04fc94660), U64(0xa28ecbd1e892abe6),
      U64(0xf1563866f5c75433), U64(0x4dae7baf70e13ed9), U64(0x7ce62ac27bd26b61),
      U64(0x70837a39109ab392), U64(0x90988e4b30b3c8ab), U64(0xb2020b63877296bf),
      U64(0x156efcb607d6675b) },
    { /* 7 */
      U64(0xe63f55ce97c331d0), U64(0x25b506b0015bba16), U64(0xc8706e29e6ad9ba8),
      U64(0x5b43d3775d521f6a), U64(0x0bfa3d577035106e), U64(0xab95fc172afb0e66),
      U64(0xf64b63979e7a3276), U64(0xf58b4562649dad4b), U64(0x48f7c3dbae0c83f1),
      U64(0xff31916642f5c8c5), U64(0xcbb048dc1c4a0495), U64(0x66b8f83cdf622989),
      U64(0x35c130e908e2b9b0), U64(0x7c761a61f0b34fa1), U64(0x3601161cf205268d),
      U64(0x9e54ccfe2219b7d6), U64(0x8b7d90a538940837), U64(0x9cd403588ea35d0b),
      U64(0xbc3c6fea9ccc5b5a), U64(0xe5ff733b6d24aeed), U64(0xceed22de0f7eb8d2),
      U64(0xec8581cab1ab545e), U64(0xb96105e88ff8e71d), U64(0x8ca03501871a5ead),
      U64(0x76ccce65d6db2a2f), U64(0x5883f582a7b58057), U64(0x3f7be4ed2e8adc3e),
      U64(0x0fe7be06355cd9c9), U64(0xee054e6c1d11be83), U64(0x1074365909b903a6),
      U64(0x5dde9f80b4813c10), U64(0x4a770c7d02b6692c), U64(0x5379c8d5d7809039),
      U64(0xb4067448161ed409), U64(0x5f5e5026183bd6cd), U64(0xe898029bf4c29df9),
      U64(0x7fb63c940a54d09c), U64(0xc5171f897f4ba8bc), U64(0xa6f28db7b31d3d72),
      U64(0x2e4f3be7716eaa78), U64(0x0d6771a099e63314), U64(0x82076254e41bf284),
      U64(0x2f0fd2b42733df98), U64(0x5c9e76d3e2dc49f0), U64(0x7aeb569619606cdb),
      U64(0x83478b07b2468764), U64(0xcfadcb8d5923cd32), U64(0x85dac7f05b95a41e),
      U64(0xb5469d1b4043a1e9), U64(0xb821ecbbd9a592fd), U64(0x1b8e0b0e798c13c8),
      U64(0x62a57b6d9a0be02e), U64(0xfcf1b793b81257f8), U64(0x9d94ea0bd8fe28eb),
      U64(0x4cea408aeb654a56), U64(0x23284a47e888996c), U64(0x2d8f1d128b893545),
      U64(0xf4cbac3132c0d8ab), U64(0xbd7c86b9ca912eba), U64(0x3a268eef3dbe6079),
      U64(0xf0d62f6077a9110c), U64(0x2735c916ade150cb), U64(0x89fd5f03942ee2ea),
      U64(0x1acee25d2fd16628), U64(0x90f39bab41181bff), U64(0x430dfe8cde39939f),
      U64(0xf70b8ac4c8274796), U64(0x1c53aeaac6024552), U64(0x13b410acf35e9c9b),
      U64(0xa532ab4249faa24f), U64(0x2b1251e5625a163f), U64(0xd7e3e676da4841c7),
      U64(0xa7b264e4e5404892), U64(0xda8497d643ae72d3), U64(0x861ae105a1723b23),
      U64(0x38a6414991048aa4), U64(0x6578dec92585b6b4), U64(0x0280cfa6acbaeadd),
      U64(0x88bdb650c273970a), U64(0x9333bd5ebbff84c2), U64(0x4e6a8f2c47dfa08b),
      U64(0x321c954db76cef2a), U64(0x418d312a72837942), U64(0xb29b38bfffcdf773),
      U64(0x6c022c38f90a4c07), U64(0x5a033a240b0f6a8a), U64(0x1f93885f3ce5da6f),
      U64(0xc38a537e96988bc6), U64(0x39e6a81ac759ff44), U64(0x29929e43cee0fce2),
      U64(0x40cdd87924de0ca2), U64(0xe9d8ebc8a29fe819), U64(0x0c2798f3cfbb46f4),
      U64(0x55e484223e53b343), U64(0x4650948ecd0d2fd8), U64(0x20e86cb2126f0651),
      U64(0x6d42c56baf5739e7), U64(0xa06fc1405ace1e08), U64(0x7babbfc54f3d193b),
      U64(0x424d17df8864e67f), U64(0xd8045870ef14980e), U64(0xc6d7397c85ac3781),
      U64(0x21a885e1443273b1), U64(0x67f8116f893f5c69), U64(0x24f5efe35706cff6),
      U64(0xd56329d076f2ab1a), U64(0x5e1eb9754e66a32d), U64(0x28d2771098bd8902),
      U64(0x8f6013f47dfdc190), U64(0x17a993fdb637553c), U64(0xe0a219397e1012aa),
      U64(0x786b9930b5da8606), U64(0x6e82e39e55b0a6da), U64(0x875a0856f72f4ec3),
      U64(0x3741ff4fa458536d), U64(0xac4859b3957558fc), U64(0x7ef6d5c75c09a57c),
      U64(0xc04a758b6c7f14fb), U64(0xf9acdd91ab26ebbf), U64(0x7391a467c5ef9668),
      U64(0x335c7c1ee1319aca), U64(0xa91533b18641e4bb), U64(0xe4bf9a683b79db0d),
      U64(0x8e20faa72ba0b470), U64(0x51f907737b3a7ae4), U64(0x2268a314bed5ec8c),
      U64(0xd944b123b949edee), U64(0x31dcb3b84d8b7017), U64(0xd3fe65279f218860),
      U64(0x097af2f1dc8ffab3), U64(0x9b09a6fc312d0b91), U64(0xcc6ded78a3c4520f),
      U64(0x3481d9ba5ebfcc50), U64(0x4f2a667f1182d56b), U64(0xdfd9fdd4509ace94),
      U64(0x26752045fbbc252b), U64(0xbffc491f662bc467), U64(0xdd593272fc202449),
      U64(0x3cbbc218d46d4303), U64(0x91b372f817456e1f), U64(0x681faf69bc6385a0),
      U64(0xb686bbeebaa43ed4), U64(0x1469b5084cd0ca01), U64(0x98c98009cbca94ac),
      U64(0x6438379a73d8c354), U64(0xc2caba2dc0c5fe26), U64(0x3e3b0dbe78d7a9de),
      U64(0x50b9ee202d670f04), U64(0x4590b27b37eab0e5), U64(0x6025b4cb36b10af3),
      U64(0xfb2c1237079c0162), U64(0xa12f28130c936be8), U64(0x4b37e52e54eb1ccc),
      U64(0x083a1ba28ad28f53), U64(0xc10a9cd83a22611b), U64(0x9f1425ad7444c236),
      U64(0x069d4cf7e9d3237a), U64(0xedc56899e7f621be), U64(0x778c273680865fcf),
      U64(0x309c5aeb1bd605f7), U64(0x8de0dc52d1472b4d), U64(0xf8ec34c2fd7b9e5f),
      U64(0xea18cd3d58787724), U64(0xaad515447ca67b86), U64(0x9989695a9d97e14c),
      U64(0x0000000000000000), U64(0xf196c63321f464ec), U64(0x71116bc169557cb5),
      U64(0xaf887f466f92c7c1), U64(0x972e3e0ffe964d65), U64(0x190ec4a8d536f915),
      U64(0x95aef1a9522ca7b8), U64(0xdc19db21aa7d51a9), U64(0x94ee18fa0471d258),
      U64(0x8087adf248a11859), U64(0xc457f6da2916dd5c), U64(0xfa6cfb6451c17482),
      U64(0xf256e0c6db13fbd1), U64(0x6a9f60cf10d96f7d), U64(0x4daaa9d9bd383fb6),
      U64(0x03c026f5fae79f3d), U64(0xde99148706c7bb74), U64(0x2a52b8b6340763df),
      U64(0x6fc20acd03edd33a), U64(0xd423c08320afdefa), U64(0xbbe1ca4e23420dc0),
      U64(0x966ed75ca8cb3885), U64(0xeb58246e0e2502c4), U64(0x055d6a021334bc47),
      U64(0xa47242111fa7d7af), U64(0xe3623fcc84f78d97), U64(0x81c744a11efc6db9),
      U64(0xaec8961539cfb221), U64(0xf31609958d4e8e31), U64(0x63e5923ecc5695ce),
      U64(0x47107ddd9b505a38), U64(0xa3afe7b5a0298135), U64(0x792b7063e387f3e6),
      U64(0x0140e953565d75e0), U64(0x12f4f9ffa503e97b), U64(0x750ce8902c3cb512),
      U64(0xdbc47e8515f30733), U64(0x1ed3610c6ab8af8f), U64(0x5239218681dde5d9),
      U64(0xe222d69fd2aaf877), U64(0xfe71783514a8bd25), U64(0xcaf0a18f4a177175),
      U64(0x61655d9860ec7f13), U64(0xe77fbc9dc19e4430), U64(0x2ccff441ddd440a5),
      U64(0x16e97aaee06a20dc), U64(0xa855dae2d01c915b), U64(0x1d1347f9905f30b2),
      U64(0xb7c652bdecf94b34), U64(0xd03e43d265c6175d), U64(0xfdb15ec0ee4f2218),
      U64(0x57644b8492e9599e), U64(0x07dda5a4bf8e569a), U64(0x54a46d71680ec6a3),
      U64(0x5624a2d7c4b42c7e), U64(0xbebca04c3076b187), U64(0x7d36f332a6ee3a41),
      U64(0x3b6667bc6be31599), U64(0x695f463aea3ef040), U64(0xad08b0e0c3282d1c),
      U64(0xb15b1e4a052a684e), U64(0x44d05b2861b7c505), U64(0x15295c5b1a8dbfe1),
      U64(0x744c01c37a61c0f2), U64(0x59c31cd1f1e8f5b7), U64(0xef45a73f4b4ccb63),
      U64(0x6bdf899c46841a9d), U64(0x3dfb2b4b823036e3), U64(0xa2ef0ee6f674f4d5),
      U64(0x184e2dfb836b8cf5), U64(0x1134df0a5fe47646), U64(0xbaa1231d751f7820),
      U64(0xd17eaa81339b62bd), U64(0xb01bf71953771dae), U64(0x849a2ea30dc8d1fe),
      U64(0x705182923f080955), U64(0x0ea757556301ac29), U64(0x041d83514569c9a7),
      U64(0x0abad4042668658e), U64(0x49b72a88f851f611), U64(0x8a3d79f66ec97dd7),
      U64(0xcd2d042bf59927ef), U64(0xc930877ab0f0ee48), U64(0x9273540deda2f122),
      U64(0xc797d02fd3f14261), U64(0xe1e2f06a284d674a), U64(0xd2be8c74c97cfd80),
      U64(0x9a494faf67707e71), U64(0xb3dbd1eca9908293), U64(0x72d14d3493b2e388),
      U64(0xd6a30f258c153427) },
};

static const STREEBOG_LONG64 C16[12][8] = {
    { U64(0xdd806559f2a64507), U64(0x05767436cc744d23), U64(0xa2422a08a460d315),
      U64(0x4b7ce09192676901), U64(0x714eb88d7585c4fc), U64(0x2f6a76432e45d016),
      U64(0xebcb2f81c0657c1f), U64(0xb1085bda1ecadae9) },
    { U64(0xe679047021b19bb7), U64(0x55dda21bd7cbcd56), U64(0x5cb561c2db0aa7ca),
      U64(0x9ab5176b12d69958), U64(0x61d55e0f16b50131), U64(0xf3feea720a232b98),
      U64(0x4fe39d460f70b5d7), U64(0x6fa3b58aa99d2f1a) },
    { U64(0x991e96f50aba0ab2), U64(0xc2b6f443867adb31), U64(0xc1c93a376062db09),
      U64(0xd3e20fe490359eb1), U64(0xf2ea7514b1297b7b), U64(0x06f15e5f529c1f8b),
      U64(0x0a39fc286a3d8435), U64(0xf574dcac2bce2fc7) },
    { U64(0x220cbebc84e3d12e), U64(0x3453eaa193e837f1), U64(0xd8b71333935203be),
      U64(0xa9d72c82ed03d675), U64(0x9d721cad685e353f), U64(0x488e857e335c3c7d),
      U64(0xf948e1a05d71e4dd), U64(0xef1fdfb3e81566d2) },
    { U64(0x601758fd7c6cfe57), U64(0x7a56a27ea9ea63f5), U64(0xdfff00b723271a16),
      U64(0xbfcd1747253af5a3), U64(0x359e35d7800fffbd), U64(0x7f151c1f1686104a),
      U64(0x9a3f410c6ca92363), U64(0x4bea6bacad474799) },
    { U64(0xfa68407a46647d6e), U64(0xbf71c57236904f35), U64(0x0af21f66c2bec6b6),
      U64(0xcffaa6b71c9ab7b4), U64(0x187f9ab49af08ec6), U64(0x2d66c4f95142a46c),
      U64(0x6fa4c33b7a3039c0), U64(0xae4faeae1d3ad3d9) },
    { U64(0x8886564d3a14d493), U64(0x3517454ca23c4af3), U64(0x06476983284a0504),
      U64(0x0992abc52d822c37), U64(0xd3473e33197a93c9), U64(0x399ec6c7e6bf87c9),
      U64(0x51ac86febf240954), U64(0xf4c70e16eeaac5ec) },
    { U64(0xa47f0dd4bf02e71e), U64(0x36acc2355951a8d9), U64(0x69d18d2bd1a5c42f),
      U64(0xf4892bcb929b0690), U64(0x89b4443b4ddbc49a), U64(0x4eb7f8719c36de1e),
      U64(0x03e7aa020c6e4141), U64(0x9b1f5b424d93c9a7) },
    { U64(0x7261445183235adb), U64(0x0e38dc92cb1f2a60), U64(0x7b2b8a9aa6079c54),
      U64(0x800a440bdbb2ceb1), U64(0x3cd955b7e00d0984), U64(0x3a7d3a1b25894224),
      U64(0x944c9ad8ec165fde), U64(0x378f5a541631229b) },
    { U64(0x74b4c7fb98459ced), U64(0x3698fad1153bb6c3), U64(0x7a1e6c303b7652f4),
      U64(0x9fe76702af69334b), U64(0x1fffe18a1b336103), U64(0x8941e71cff8a78db),
      U64(0x382ae548b2e4f3f3), U64(0xabbedea680056f52) },
    { U64(0x6bcaa4cd81f32d1b), U64(0xdea2594ac06fd85d), U64(0xefbacd1d7d476e98),
      U64(0x8a1d71efea48b9ca), U64(0x2001802114846679), U64(0xd8fa6bbbebab0761),
      U64(0x3002c6cd635afe94), U64(0x7bcd9ed0efc889fb) },
    { U64(0x48bc924af11bd720), U64(0xfaf417d5d9b21b99), U64(0xe71da4aa88e12852),
      U64(0x5d80ef9d1891cc86), U64(0xf82012d430219f9b), U64(0xcda43c32bcdf1d77),
      U64(0xd21380b00449b17a), U64(0x378ee767f11631ba) },
};

#define B(x, i, j)                                                                 \
    (((STREEBOG_LONG64)(*(((const uint8_t *)(&x)) + i))) << (j * 8))
#define PULL64(x)                                                                  \
    (B(x, 0, 0) | B(x, 1, 1) | B(x, 2, 2) | B(x, 3, 3) | B(x, 4, 4) | B(x, 5, 5)   \
     | B(x, 6, 6) | B(x, 7, 7))
#define SWAB64(x)                                                                  \
    (B(x, 0, 7) | B(x, 1, 6) | B(x, 2, 5) | B(x, 3, 4) | B(x, 4, 3) | B(x, 5, 2)   \
     | B(x, 6, 1) | B(x, 7, 0))

static inline STREEBOG_LONG64 multipermute(const STREEBOG_LONG64 *in, int i)
{
    STREEBOG_LONG64 t = 0;

    t ^= A_PI_table[0][(in[0] >> (i * 8)) & 0xff];
    t ^= A_PI_table[1][(in[1] >> (i * 8)) & 0xff];
    t ^= A_PI_table[2][(in[2] >> (i * 8)) & 0xff];
    t ^= A_PI_table[3][(in[3] >> (i * 8)) & 0xff];
    t ^= A_PI_table[4][(in[4] >> (i * 8)) & 0xff];
    t ^= A_PI_table[5][(in[5] >> (i * 8)) & 0xff];
    t ^= A_PI_table[6][(in[6] >> (i * 8)) & 0xff];
    t ^= A_PI_table[7][(in[7] >> (i * 8)) & 0xff];

    return t;
}

static void transform(STREEBOG_LONG64 *out, const STREEBOG_LONG64 *a,
                      const STREEBOG_LONG64 *b)
{
    STREEBOG_LONG64 tmp[8];

    tmp[0] = a[0] ^ b[0];
    tmp[1] = a[1] ^ b[1];
    tmp[2] = a[2] ^ b[2];
    tmp[3] = a[3] ^ b[3];
    tmp[4] = a[4] ^ b[4];
    tmp[5] = a[5] ^ b[5];
    tmp[6] = a[6] ^ b[6];
    tmp[7] = a[7] ^ b[7];

    out[0] = multipermute(tmp, 0);
    out[1] = multipermute(tmp, 1);
    out[2] = multipermute(tmp, 2);
    out[3] = multipermute(tmp, 3);
    out[4] = multipermute(tmp, 4);
    out[5] = multipermute(tmp, 5);
    out[6] = multipermute(tmp, 6);
    out[7] = multipermute(tmp, 7);
}

static inline void gN(STREEBOG_LONG64 *h, STREEBOG_LONG64 *m, STREEBOG_LONG64 *N)
{
    STREEBOG_LONG64 K[8];
    STREEBOG_LONG64 T[8];
    int i;

    transform(K, h, N);

    transform(T, K, m);
    transform(K, K, C16[0]);
    for (i = 1; i < 12; i++) {
        transform(T, K, T);
        transform(K, K, C16[i]);
    }

    h[0] ^= T[0] ^ K[0] ^ m[0];
    h[1] ^= T[1] ^ K[1] ^ m[1];
    h[2] ^= T[2] ^ K[2] ^ m[2];
    h[3] ^= T[3] ^ K[3] ^ m[3];
    h[4] ^= T[4] ^ K[4] ^ m[4];
    h[5] ^= T[5] ^ K[5] ^ m[5];
    h[6] ^= T[6] ^ K[6] ^ m[6];
    h[7] ^= T[7] ^ K[7] ^ m[7];
}

static void streebog_single_block(STREEBOG_CTX *ctx, const uint8_t *in,
                                  size_t num)
{
    STREEBOG_LONG64 M[8], l;
    int i;

    for (i = 0; i < 8; i++)
        M[i] = PULL64(in[i * 8]);

    gN(ctx->h, M, ctx->N);

    l = ctx->N[0];
    ctx->N[0] += num;

    if (ctx->N[0] < l || ctx->N[0] < num) {
        for (i = 1; i < 8; i++) {
            ctx->N[i]++;
            if (ctx->N[i] != 0)
                break;
        }
    }

    ctx->Sigma[0] += M[0];
    for (i = 1; i < 8; i++)
        if (ctx->Sigma[i - 1] < M[i - 1])
            ctx->Sigma[i] += M[i] + 1;
        else
            ctx->Sigma[i] += M[i];
}

static void streebog_block_data_order(STREEBOG_CTX *ctx, const uint8_t *in, size_t num)
{
    int i;
    for (i = 0; i < num; i++)
        streebog_single_block(ctx, in + i * STREEBOG_CBLOCK, 64 * 8);
}

int STREEBOG512_Final(uint8_t *md, STREEBOG_CTX *c)
{
    int n;
    uint8_t *p = (uint8_t *)c->data;
    STREEBOG_LONG64 Z[STREEBOG_LBLOCK] = {};

    if (c->num == STREEBOG_CBLOCK) {
        streebog_block_data_order(c, p, 1);
        c->num -= STREEBOG_CBLOCK;
    }

    n = c->num;
    p[n++] = 1;
    memset(p + n, 0, STREEBOG_CBLOCK - n);

    streebog_single_block(c, p, c->num * 8);

    gN(c->h, c->N, Z);
    gN(c->h, c->Sigma, Z);

    for (n = 0; n < STREEBOG_LBLOCK; n++)
        c->h[n] = SWAB64(c->h[n]);

    if (md == 0)
        return 0;

    switch (c->md_len) {
        /* Let compiler decide if it's appropriate to unroll... */
        case STREEBOG256_LENGTH:
            for (n = 0; n < STREEBOG256_LENGTH / 8; n++) {
                STREEBOG_LONG64 t = c->h[4 + n];

#if BYTE_ORDER == BIG_ENDIAN
                *(md++) = (uint8_t)(t);
                *(md++) = (uint8_t)(t >> 8);
                *(md++) = (uint8_t)(t >> 16);
                *(md++) = (uint8_t)(t >> 24);
                *(md++) = (uint8_t)(t >> 32);
                *(md++) = (uint8_t)(t >> 40);
                *(md++) = (uint8_t)(t >> 48);
                *(md++) = (uint8_t)(t >> 56);
#else
                *(md++) = (uint8_t)(t >> 56);
                *(md++) = (uint8_t)(t >> 48);
                *(md++) = (uint8_t)(t >> 40);
                *(md++) = (uint8_t)(t >> 32);
                *(md++) = (uint8_t)(t >> 24);
                *(md++) = (uint8_t)(t >> 16);
                *(md++) = (uint8_t)(t >> 8);
                *(md++) = (uint8_t)(t);
#endif
            }
            break;
        case STREEBOG512_LENGTH:
            for (n = 0; n < STREEBOG512_LENGTH / 8; n++) {
                STREEBOG_LONG64 t = c->h[n];

#if BYTE_ORDER == BIG_ENDIAN
                *(md++) = (uint8_t)(t);
                *(md++) = (uint8_t)(t >> 8);
                *(md++) = (uint8_t)(t >> 16);
                *(md++) = (uint8_t)(t >> 24);
                *(md++) = (uint8_t)(t >> 32);
                *(md++) = (uint8_t)(t >> 40);
                *(md++) = (uint8_t)(t >> 48);
                *(md++) = (uint8_t)(t >> 56);
#else
                *(md++) = (uint8_t)(t >> 56);
                *(md++) = (uint8_t)(t >> 48);
                *(md++) = (uint8_t)(t >> 40);
                *(md++) = (uint8_t)(t >> 32);
                *(md++) = (uint8_t)(t >> 24);
                *(md++) = (uint8_t)(t >> 16);
                *(md++) = (uint8_t)(t >> 8);
                *(md++) = (uint8_t)(t);
#endif
            }
            break;
        /* ... as well as make sure md_len is not abused. */
        default:
            return 0;
    }

    return 1;
}

int STREEBOG256_Final(uint8_t *md, STREEBOG_CTX *c)
{
    return STREEBOG512_Final(md, c);
}

int STREEBOG512_Update(STREEBOG_CTX *c, const void *_data, size_t len)
{
    uint8_t *p = (uint8_t *)c->data;
    const uint8_t *data = (const uint8_t *)_data;

    if (len == 0)
        return 1;

    if (c->num != 0) {
        size_t n = STREEBOG_CBLOCK - c->num;

        if (len < n) {
            memcpy(p + c->num, data, len);
            c->num += (unsigned int)len;
            return 1;
        } else {
            memcpy(p + c->num, data, n);
            c->num = 0;
            len -= n;
            data += n;
            streebog_block_data_order(c, p, 1);
        }
    }

    if (len >= STREEBOG_CBLOCK) {
        streebog_block_data_order(c, data, len / STREEBOG_CBLOCK);
        data += len;
        len %= STREEBOG_CBLOCK;
        data -= len;
    }

    if (len != 0) {
        memcpy(p, data, len);
        c->num = (int)len;
    }

    return 1;
}

int STREEBOG256_Update(STREEBOG_CTX *c, const void *data, size_t len)
{
    return STREEBOG512_Update(c, data, len);
}

void STREEBOG512_Transform(STREEBOG_CTX *c, const uint8_t *data)
{
    streebog_block_data_order(c, data, 1);
}

int STREEBOG256_Init(STREEBOG_CTX *c)
{
    memset(c, 0, sizeof(*c));
    memset(c->h, 1, sizeof(c->h));

    c->md_len = STREEBOG256_LENGTH;
    return 1;
}

int STREEBOG512_Init(STREEBOG_CTX *c)
{
    memset(c, 0, sizeof(*c));
    memset(c->h, 0, sizeof(c->h));

    c->num = 0;
    c->md_len = STREEBOG512_LENGTH;
    return 1;
}

uint8_t *STREEBOG256(const uint8_t *d, size_t n, uint8_t *md)
{
    STREEBOG_CTX c;
    static uint8_t m[STREEBOG256_LENGTH];

    if (md == NULL)
        md = m;
    STREEBOG256_Init(&c);
    STREEBOG256_Update(&c, d, n);
    STREEBOG256_Final(md, &c);
    OPENSSL_cleanse(&c, sizeof(c));
    return (md);
}

uint8_t *STREEBOG512(const uint8_t *d, size_t n, uint8_t *md)
{
    STREEBOG_CTX c;
    static uint8_t m[STREEBOG512_LENGTH];

    if (md == NULL)
        md = m;
    STREEBOG512_Init(&c);
    STREEBOG512_Update(&c, d, n);
    STREEBOG512_Final(md, &c);
    OPENSSL_cleanse(&c, sizeof(c));
    return (md);
}

#endif
