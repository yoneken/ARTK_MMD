######################################################################################
# License
######################################################################################

# This file is part of the ARTK_MMD Project.
# ---------------------------------------------------------------------------------------
# このプログラムはフリーソフトウェアです。あなたはこれを、フリーソフトウェア財団
# によって発行された GNU 一般公衆利用許諾契約書(バージョン2か、希望によっては
# それ以降のバージョンのうちどれか)の定める条件の下で再頒布または改変することができます。
#
# このプログラムは有用であることを願って頒布されますが、*全くの無保証* です。
# 商業可能性の保証や特定の目的への適合性は、言外に示されたものも含め
# 全く存在しません。詳しくは同梱のGNU 一般公衆利用許諾契約書をご覧ください。
# ---------------------------------------------------------------------------------------

######################################################################################
# CMake directives
######################################################################################

#Require 2.6 or higher. 

cmake_minimum_required(VERSION 2.6)

######################################################################################
# Project declaration and options
######################################################################################

PROJECT(ARTK_MMD)

SET( PROJECT_VER_MAJOR 0 )
SET( PROJECT_VER_MINOR 1 )
SET( PROJECT_VER_PATCH 0 )
SET( PROJECT_VER 
  "${PROJECT_VER_MAJOR}.${PROJECT_VER_MINOR}.${PROJECT_VER_PATCH}")

FIND_PACKAGE( GLUT REQUIRED )
FIND_PACKAGE( OPENGL REQUIRED )
INCLUDE_DIRECTORIES( ${GLUT_INCLUDE_DIR} )

IF( APPLE )
  SET( CMAKE_OSX_ARCHITECTURES "i386" CACHE STRING "Build architectures for OSX" FORCE) # ARToolKit don't support for x86_64.
  FIND_PACKAGE( QUICKTIME REQUIRED )
  LINK_DIRECTORIES( ${QUICKTIME_LIBRARY} )
ENDIF( APPLE )

find_package( BULLET REQUIRED )
IF( BULLET_FOUND )
  INCLUDE_DIRECTORIES( ${BULLET_INCLUDE_DIR} )
ELSE( BULLET_FOUND )
  SET( BULLET_INCLUDE_DIR CACHE PATH "Path to a include directory of Bullet" )
  INCLUDE_DIRECTORIES( ${BULLET_INCLUDE_DIR} )
  SET( BULLET_LIB_DIR CACHE PATH "Path to a lib directory of Bullet" )
  LINK_DIRECTORIES( ${BULLET_LIB_DIR} )
ENDIF( BULLET_FOUND )

SET( TBB_INCLUDE_DIR CACHE PATH "Path to a include directory of Intel Threading Building Block" )
INCLUDE_DIRECTORIES( ${TBB_INCLUDE_DIR} )
SET( TBB_LIB_DIR CACHE PATH "Path to a lib directory of Intel Threading Building Block" )
LINK_DIRECTORIES( ${TBB_LIB_DIR} )

SET( ARTOOLKIT_INCLUDE_DIR CACHE PATH "Path to a include directory of ARToolKit" )
INCLUDE_DIRECTORIES( ${ARTOOLKIT_INCLUDE_DIR} )
SET( ARTOOLKIT_LIB_DIR CACHE PATH "Path to a lib directory of ARToolKit" )
LINK_DIRECTORIES( ${ARTOOLKIT_LIB_DIR} )

ADD_EXECUTABLE(ARTK_MMD src/main.cpp src/ErrorMes.cpp 
  src/ARTK/ARTK.cpp 
  src/BulletPhysics/BulletPhysics.cpp 
  src/MMD/MotionPlayer.cpp src/MMD/PMDBone.cpp src/MMD/PMDConstraint.cpp 
  src/MMD/PMDFace.cpp src/MMD/PMDIK.cpp src/MMD/PMDModel.cpp
  src/MMD/PMDRigidBody.cpp src/MMD/TextureList.cpp src/MMD/VecMatQuat.cpp 
  src/MMD/VMDBezier.cpp src/MMD/VMDMotion.cpp )

TARGET_LINK_LIBRARIES( ARTK_MMD ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} )

IF( APPLE )
  TARGET_LINK_LIBRARIES( ARTK_MMD ${QUICKTIME_LIBRARY} )
  TARGET_LINK_LIBRARIES( ARTK_MMD "-framework Carbon" )
ENDIF( APPLE )

IF( BULLET_FOUND )
  TARGET_LINK_LIBRARIES( ARTK_MMD ${BULLET_LIBRARIES} )
ELSE( BULLET_FOUND )
  TARGET_LINK_LIBRARIES( ARTK_MMD ${BULLET_LIB_DIR} BulletMultiThreaded BulletCollision BulletDynamics BulletSoftBody LinearMath )
ENDIF (BULLET_FOUND )

#TARGET_LINK_LIBRARIES( ARTK_MMD ${TBB_LIB_DIR} )
TARGET_LINK_LIBRARIES( ARTK_MMD ${ARTOOLKIT_LIB_DIR} ARgsub_lite ARgsubUtil ARMulti ARvideo AR)
