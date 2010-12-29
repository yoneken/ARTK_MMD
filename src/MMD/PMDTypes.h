//***************************
// PMDモデル用各種構造体定義
//***************************

// この構造体定義は、ナクアダ氏(http://www.geocities.jp/hatsune_no_mirai/)をはじめとする
// 「MMDのモデルデータについて語るスレ」(http://jbbs.livedoor.jp/bbs/read.cgi/music/23040/1219738115/)
// に書き込まれたPMDファイルフォーマット解析情報と、Ｙｕｍｉｎ氏(http://yumin3123.at.webry.info/)の
// VMDConverterのソースファイルを参考にさせていただきました

#ifndef	_PMDTYPES_H_
#define	_PMDTYPES_H_

#include	"VecMatQuat.h"

#pragma pack( push, 1 )

// ファイルヘッダ
struct PMD_Header
{
	char	szMagic[3];		// "Pmd"
	float	fVersion;		// PMDバージョン番号
	char	szName[20];		// モデル名
	char	szComment[256];	// コメント(著作権表示など)
};

// 頂点データ
struct PMD_Vertex
{
	Vector3		vec3Pos;	// 座標
	Vector3		vec3Normal;	// 法線ベクトル
	TexUV		uvTex;		// テクスチャ座標

	unsigned short	unBoneNo[2];	// ボーン番号
	unsigned char	cbWeight;		// ブレンドの重み (0～100％)
	unsigned char	cbEdge;			// エッジフラグ
};

// マテリアルデータ
struct PMD_Material
{
	Color4		col4Diffuse;
	float		fShininess;
	Color3		col3Specular,
				col3Ambient;

	unsigned short	unknown;
	unsigned int	ulNumIndices;			// この材質に対応する頂点数
	char			szTextureFileName[20];	// テクスチャファイル名
};

// ボーンデータ
struct PMD_Bone
{
	char	szName[20];			// ボーン名 (0x00 終端，余白は 0xFD)
	short	nParentNo;			// 親ボーン番号 (なければ -1)
	short	nChildNo;			// 子ボーン番号

	unsigned char	cbKind;		// ボーンの種類
	unsigned short	unIKTarget;	// IK時のターゲットボーン

	Vector3		vec3Position;	// モデル原点からの位置
};
/*
kind
	0x00　通常ボーン(回転のみ)
	0x01　通常ボーン(移動可能)
	0x02　IKボーン(移動可能)
	0x03　見えない(選択可)
	0x04　IKボーン(回転のみ)
	0x05　回転追従(回転のみ)
	0x06　IK接続先
	0x07　見えない(選択不可)
	0x08　ひねり
	0x09　回転運動
*/

// IKデータ
struct PMD_IK
{
	short			nTargetNo;	// IKターゲットボーン番号
	short			nEffNo;		// IK先端ボーン番号

	unsigned char	cbNumLink;	// IKを構成するボーンの数

	unsigned short	unCount;
	float			fFact;

	unsigned short	punLinkNo[1];// IKを構成するボーンの配列
};

// 表情頂点
struct PMD_FaceVtx
{
	unsigned int	ulIndex;
	Vector3			vec3Pos;
};

// 表情データ
struct PMD_Face
{
	char			szName[20];		// 表情名 (0x00 終端，余白は 0xFD)

	unsigned int	ulNumVertices;	// 表情頂点数
	unsigned char	cbType;			// 分類

	PMD_FaceVtx		pVertices[1];	// 表情頂点データ
};

// 剛体データ
struct PMD_RigidBody
{
	char			szName[20];		// 剛体名

	unsigned short	unBoneIndex;	// 関連ボーン番号
	unsigned char	cbColGroupIndex;// 衝突グループ
	unsigned short	unColGroupMask;	// 衝突グループマスク

	unsigned char	cbShapeType;	// 形状  0:球 1:箱 2:カプセル

	float			fWidth;			// 半径(幅)
	float			fHeight;		// 高さ
	float			fDepth;			// 奥行

	Vector3			vec3Position;	// 位置(ボーン相対)
	Vector3			vec3Rotation;	// 回転(radian)

	float			fMass;			// 質量
	float			fLinearDamping;	// 移動減
	float			fAngularDamping;// 回転減
	float			fRestitution;	// 反発力
	float			fFriction;		// 摩擦力

	unsigned char	cbRigidBodyType;// タイプ 0:Bone追従 1:物理演算 2:物理演算(Bone位置合せ)
};

// コンストレイント(ジョイント)データ
struct PMD_Constraint
{
	char			szName[20];		// コンストレイント名

	unsigned int	ulRigidA;		// 剛体A
	unsigned int	ulRigidB;		// 剛体B

	Vector3			vec3Position;	// 位置(モデル原点中心)
	Vector3			vec3Rotation;	// 回転(radian)

	Vector3			vec3PosLimitL;	// 移動制限1
	Vector3			vec3PosLimitU;	// 移動制限2

	Vector3			vec3RotLimitL;	// 回転制限1
	Vector3			vec3RotLimitU;	// 回転制限2

	Vector3			vec3SpringPos;	// ばね移動
	Vector3			vec3SpringRot;	// ばね回転
};

#pragma pack( pop )

#endif	// _PMDTYPES_H_
