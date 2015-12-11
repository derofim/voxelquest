// f00375_gamephysics.h
//

#include "f00375_gamephysics.e"
#define LZZ_INLINE inline
GamePhysics::GamePhysics ()
                      {
		lastBodyPick = NULL;
		//gameActor = NULL;
		//8000; // ~120 times per second
	}
void GamePhysics::init (Singleton * _singleton)
        {
		totTime = 0.0;
		BASE_ENT_HEIGHT = 2.0f;
		
		cout << "GamePhysics:init()\n";
		
		singleton = _singleton;
		myOGLApp = new MyOGLApp("yo", 640, 480);
		guiHelper = new MyGLHelper(singleton, myOGLApp);
		example = new BenchmarkDemo(guiHelper,5);
		example->initPhysics();
		
	}
void GamePhysics::pickBody (FIVector4 * mouseDownOPD)
                                               { //btVector3 posWS1, btVector3 posWS2) {
		
		if (!(singleton->editPose)) {
			lastBodyPick = NULL;
			return;
		}
		
		int bodyUID = mouseDownOPD->getFW();
		int limbUID = mouseDownOPD->getFZ();
		BaseObj* ge;
		
		if (
			(bodyUID > 0) &&
			(limbUID > -1)	
		) {
			ge = &(singleton->gw->gameObjects[bodyUID]);
			lastBodyPick = ge->bodies[limbUID].body;
		}
		else {
			lastBodyPick = NULL;
		}
		
		
		// btVector3 begPos = btVector3(0.0f,0.0f,0.0f);
		// btVector3 endPos = btVector3(0.0f,0.0f,0.0f);
		// btVector3 rayDir = btVector3(0.0f,0.0f,0.0f);
		
		// singleton->getRay(
		// 	singleton->lastMouseX,
		// 	singleton->lastMouseY,
		// 	begPos,
		// 	endPos,
		// 	rayDir
		// );
		
		
		// btVector3 begPos = singleton->cameraGetPosNoShake()->getBTV();
		// btVector3 endPos;
		
		// if (posWS1.distance(begPos) < posWS2.distance(begPos)) {
		// 	endPos = posWS1;
		// }
		// else {
		// 	endPos = posWS2;
		// }
		
		// lastBodyPick = example->bodyPick(begPos,endPos);
		
		
		// singleton->getRayTo(
		// 	singleton->lastMouseX,
		// 	singleton->lastMouseY
		// );
		
		
		
		// if (lastBodyPick != NULL) {
		// 	cout << "objID " << lastBodyPick->bodyUID << "\n";
		// 	cout << "limbUID " << lastBodyPick->limbUID << "\n\n";
		// }
		
		
	}
void GamePhysics::collectDebris ()
                             {
		int i;
		FIVector4 tempVec;
		int entNum;
		
		for (i = 0; i < singleton->debrisStack.size(); i++) {
			
			tempVec.setBTV(singleton->debrisStack[i].pos);
			entNum = singleton->placeNewEnt(false, E_ENTTYPE_DEBRIS, &tempVec);
			
			//addDebris(singleton->debrisStack[i].pos);
		}
		singleton->debrisStack.clear();
	}
void GamePhysics::beginDrop ()
                         {
		
		cout << "GamePhysics:beginDrop()\n";
		
		example->beginDrop(
			singleton->cameraGetPosNoShake()->getFX(),
			singleton->cameraGetPosNoShake()->getFY(),
			singleton->cameraGetPosNoShake()->getFZ()
		);
	}
void GamePhysics::remBoxFromObj (BaseObjType _uid)
                                             {
		
		BaseObj* ge = &(singleton->gw->gameObjects[_uid]);
		
		int bodInd;
		for (bodInd = 0; bodInd < ge->bodies.size(); bodInd++) {
			example->removeRigidBody(ge->bodies[bodInd].body);
		}
		ge->bodies.clear();
	}
void GamePhysics::addBoxFromObj (BaseObjType _uid)
                                             {
		
		int i;
		int curOrgType;
		GameOrg* curOrg = NULL;
		
		BaseObj* ge = &(singleton->gw->gameObjects[_uid]);
		
		if (ge->isHidden) {
			return;
		}
		
		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(ge->startPoint);
		
		GameActor* curActor;
		
		float objRad = 1.0f;
		
		
		
		bool isOrg = false;
		
		switch (ge->entType) {
			case E_ENTTYPE_NPC:
			case E_ENTTYPE_MONSTER:
			case E_ENTTYPE_WEAPON:
				{
					
					
					isOrg = true;
					
					
					
					
					if (ge->entType == E_ENTTYPE_WEAPON) {
						curOrgType = E_ORGTYPE_WEAPON;
					}
					else {
						curOrgType = E_ORGTYPE_HUMAN;
						
						btCapsuleShapeZ* capsuleShapeZ = new btCapsuleShapeZ(1.0f,BASE_ENT_HEIGHT);
						ge->bodies.push_back(BodyStruct());
						ge->bodies.back().body = example->createRigidBodyMask(
							MASS_PER_LIMB, // 0.1
							trans,
							capsuleShapeZ,
							COL_MARKER,
							markerCollidesWith
						);
						ge->bodies.back().body->setAngularFactor(btVector3(0.0f,0.0f,0.0f));
						ge->bodies.back().boneId = -1;
						
					}
					
					
					singleton->gameOrgs.push_back(new GameOrg());
					singleton->gameOrgs.back()->init(singleton, ge->uid,curOrgType);
					ge->orgId = singleton->gameOrgs.size()-1;
					
					singleton->gameActors.push_back(new GameActor(
						singleton,
						ge->uid,
						example->getWorld(),
						ge->startPoint,
						false	
					));
					
					curActor = (singleton->gameActors.back());
					ge->actorId = singleton->gameActors.size()-1;
					
					for (i = 0; i < curActor->actorJoints.size(); i++) {
						
						
						ge->bodies.push_back(BodyStruct());
						ge->bodies.back().body = curActor->actorJoints[i].body;
						ge->bodies.back().boneId = curActor->actorJoints[i].boneId;
						ge->bodies.back().isBall = curActor->actorJoints[i].isBall;
						
						// if (i == 0) {
						// 	//ge->body = curActor->actorJoints[i].body;
						// 	//ge->body->setLinearFactor(btVector3(0.0f,0.0f,0.0f));
						// 	ge->bodies.back().body->setAngularFactor(btVector3(0.0f,0.0f,0.0f));
						// }
						// else {
							
						// }
					}
				}
			break;
			// case E_ENTTYPE_WEAPON:
			// 	{
			// 		btCapsuleShapeX* capsuleShapeX = new btCapsuleShapeX(0.25f,2.0f);
					
			// 		ge->bodies.push_back(BodyStruct());
			// 		ge->bodies.back().body = example->createRigidBodyMask(
			// 			MASS_PER_LIMB,
			// 			trans,
			// 			capsuleShapeX
			// 			,COL_WEAPON,
			// 			weaponCollidesWith
			// 		);
					
			// 		ge->bodies[0].boneId = -1;
			// 		ge->bodies[0].body->setDamping(0.99f,0.9f);
			// 	}
			// break;
			default:
				{
					btBoxShape* boxShape = new btBoxShape(btVector3(objRad,objRad,objRad));
					ge->bodies.push_back(BodyStruct());
					ge->bodies.back().body = example->createRigidBodyMask(
						MASS_PER_LIMB,
						trans,
						boxShape
						,COL_DYN,
						dynCollidesWith
					);
					ge->bodies[0].boneId = -1;
					
					if (ge->entType == E_ENTTYPE_DEBRIS) {
						
						ge->bodies[0].body->setAngularVelocity(btVector3(
							(fGenRand2()*2.0f-1.0f),
							(fGenRand2()*2.0f-1.0f),
							(fGenRand2()*2.0f-1.0f)	
						)*4.0f);
					}
					
					
				}
			break;
		}
		
		
		int bodInd;
		for (bodInd = 0; bodInd < ge->bodies.size(); bodInd++) {
			ge->bodies[bodInd].body->bodyUID = _uid;
			ge->bodies[bodInd].body->limbUID = bodInd;
			
			
			switch(ge->bodies[bodInd].boneId) {
				
				case E_BONE_L_UPPERARM:
				case E_BONE_L_LOWERARM:
				case E_BONE_L_METACARPALS:
				case E_BONE_L_UPPERLEG:
				case E_BONE_L_LOWERLEG:
				case E_BONE_L_TALUS:
				
				case E_BONE_R_UPPERARM:
				case E_BONE_R_LOWERARM:
				case E_BONE_R_METACARPALS:
				case E_BONE_R_UPPERLEG:
				case E_BONE_R_LOWERLEG:
				case E_BONE_R_TALUS:
					ge->bodies[bodInd].body->setDamping(0.99f,0.9f);
				break;
				case -1:
				case E_BONE_WEAPON_BASE:
				case E_BONE_WEAPON_END:
				case E_BONE_WEAPON_0:
				case E_BONE_WEAPON_1:
				case E_BONE_WEAPON_2:
				case E_BONE_WEAPON_3:
				case E_BONE_WEAPON_4:
				case E_BONE_WEAPON_5:
				case E_BONE_WEAPON_6:
				case E_BONE_WEAPON_7:
				case E_BONE_WEAPON_8:
					ge->bodies[bodInd].body->setDamping(0.1f,0.9f);
				break;
				
				default:
					// linear, angular
					ge->bodies[bodInd].body->setDamping(0.999f,0.9f);
				break;
			}
			
			
			
			
			ge->bodies[bodInd].body->setContactProcessingThreshold(0.1f);
			
			ge->bodies[bodInd].isVisible = true;
			
			if (isOrg) {
				
				curOrg = singleton->gameOrgs[ge->orgId];
				if (curOrg->orgType == E_ORGTYPE_HUMAN) {
					if (
						(bodInd == 0)
						|| (
							(ge->bodies[bodInd].boneId == E_BONE_C_BASE) &&
							(!(ge->bodies[bodInd].isBall))	
						)
					) {
						ge->bodies[bodInd].isVisible = false;
					}
					
					//ge->bodies[bodInd].isVisible = false;
				}
				
				if (curOrg->orgType == E_ORGTYPE_WEAPON) {
					if (ge->bodies[bodInd].isBall || (ge->bodies[bodInd].boneId == E_BONE_WEAPON_BASE) ) {
						ge->bodies[bodInd].isVisible = false;
					}
				}
				
				
			}
			
			ge->bodies[bodInd].mass = MASS_PER_LIMB;
			ge->bodies[bodInd].hasContact = false;
			ge->bodies[bodInd].isInside = false;
			ge->bodies[bodInd].isFalling = true;
			ge->bodies[bodInd].inWater = false;
			ge->bodies[bodInd].lastVel = btVector3(0.0f,0.0f,0.0f);
			ge->bodies[bodInd].totAV = btVector3(0.0f,0.0f,0.0f);
			ge->bodies[bodInd].totLV = btVector3(0.0f,0.0f,0.0f);
			
			
		}
		
		
		
	}
void GamePhysics::flushImpulses ()
                             {
		
		int k;
		BaseObj* ge;
		
		for(k = 0; k < singleton->gw->visObjects.size(); k++) {
			ge = &(singleton->gw->gameObjects[singleton->gw->visObjects[k]]);
			
			if (
				(ge->isHidden)
			) {
				
			}
			else {
				ge->flushImpulses();
			}
		}
	}
void GamePhysics::collideWithWorld (double curStepTime)
                                                  {
		
		
		int i;
		int j;
		int k;
		int m;
		
		int bodInd;
		int cellVal;
		BodyStruct* curBody;
		
		BaseObj* ge;
		BaseObj* grabber;
		
		FIVector4* curCenterPoint;
		btDiscreteDynamicsWorld* world = example->getWorld();
		
		btVector3 tempBTV;
		btVector3 tempBTV2;
		btVector3 nv0;
		btVector3 nv1;
		
		
		bool hasContact = false;
		
		int entNum;
		
		FIVector4 tempVec;
		
		
		collectDebris();
		
		
		
		
		for(k = 0; k < singleton->gw->visObjects.size(); k++) {
			ge = &(singleton->gw->gameObjects[singleton->gw->visObjects[k]]);
			
			if (
				(ge->isHidden)
			) {
				
			}
			else {
				
				for (bodInd = 0; bodInd < ge->bodies.size(); bodInd++) {
					ge->bodies[bodInd].hasContact = false;
					ge->bodies[bodInd].isFalling = true;
				}
				
			}
		}
		
		
		const btCollisionObject* bodies[2];
		
		int numManifolds = world->getDispatcher()->getNumManifolds();
		for (i=0;i<numManifolds;i++) {
			btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);
			
			
			
			const btCollisionObject* obA = (contactManifold->getBody0());
			const btCollisionObject* obB = (contactManifold->getBody1());

			bodies[0] = obA;
			bodies[1] = obB;

			hasContact = false;
			
			int numContacts = contactManifold->getNumContacts();
			for (j=0;j<numContacts;j++) {
				btManifoldPoint& pt = contactManifold->getContactPoint(j);
				if (pt.getDistance() < 0.1f) {
					
					hasContact = true;
					
					// const btVector3& ptA = pt.getPositionWorldOnA();
					// const btVector3& ptB = pt.getPositionWorldOnB();
					// const btVector3& normalOnB = pt.m_normalWorldOnB;
				}
			}
			
			
			if (bodies[0]->bodyUID == bodies[1]->bodyUID) {
				// don't register contacts within same entity
			}
			else {
				for (k = 0; k < 2; k++) {
					if (
						(bodies[k]->bodyUID > -1) &&
						(bodies[k]->limbUID > -1)
					) {
						ge = &(singleton->gw->gameObjects[ bodies[k]->bodyUID ]);
						curBody =  &(ge->bodies[ bodies[k]->limbUID ]);
						
						
						if (
							(ge->isHidden)
						) {
							
						}
						else {
							
							if (hasContact) {
								curBody->hasContact = true;
								curBody->isFalling = false;
							}
							
							
						}
						
					}
				}
			}
			
			
			
		}
		
		
		
		float totMass;
		float totForce;
		btVector3 dirForce;
		GameOrg* curOrg = NULL;
		GameOrgNode* curOrgNode = NULL;
		GameActor* curActor = NULL;
		btVector3 basePos;
		
		btVector3 difVec = btVector3(0.0,0.0,0.0);
		btVector3 totVec = btVector3(0.0,0.0,0.0);
		bool hasRig = false;
		bool animatedRig = false;
		bool doProc = false;
		
		for(k = 0; k < singleton->gw->visObjects.size(); k++) {
			ge = &(singleton->gw->gameObjects[singleton->gw->visObjects[k]]);
			
			hasRig = false;
			animatedRig = false;
			
			if (
				(ge->isHidden)
			) {
				
			}
			else {
				
				
				
				hasRig = (
					(ge->orgId > -1) &&
					(ge->actorId > -1)	
				);
				
				
				
				totVec = btVector3(0.0,0.0,0.0);
				
				
				if (hasRig) {
					curActor = singleton->gameActors[ge->actorId];
					curOrg = singleton->gameOrgs[ge->orgId];
					animatedRig = (curOrg->orgType == E_ORGTYPE_HUMAN);
					
					
					if (animatedRig) {
						
						if (ge->baseContact()) {
							
							// ge->contactCount++;
							
							// if (ge->contactCount > 50) {
							// 	//ge->isJumping = false;
							// }
							
							//if (ge->bodies[0].body->getLinearVelocity().getZ() < 0.01) {
								
							//}
							
							ge->bodies[0].body->setGravity(btVector3(0.0f,0.0f,-2.0f));
						}
						else {
							
							//ge->contactCount = 0;
							
							ge->bodies[0].body->setGravity(btVector3(0.0f,0.0f,-10.0f));
						}
						
						if (ge->isJumping) {
							curOrg->targetPoseGroup = E_PG_JUMP;
						}
						else {
							
							if (singleton->isWalking || (ge->getPlanarVel() > 0.1f)) { //ge->bodies[0].body->getLinearVelocity().length() > 0.1f
								curOrg->targetPoseGroup = E_PG_WALK;
							}
							else {
								curOrg->targetPoseGroup = E_PG_IDLE;
							}
							
						}
						
						//ge->isJumping = (curOrg->targetPoseGroup == E_PG_JUMP);
						
						
						
						curOrg->updatePose(curStepTime);
						
						if (ge->isGrabbingId > -1) {
							ge->updateWeapon(
								curStepTime,
								// clampfZO(
								// 	(singleton->lastMouseX-singleton->bufferDim.getFX()*0.25f)
								// 	*4.0f/singleton->bufferDim.getFX()),
								// clampfZO(
								// 	(singleton->lastMouseY-singleton->bufferDim.getFY()*0.25f)
								// 	*4.0f/singleton->bufferDim.getFY()),
								
								2.0f // todo: update this to actual weapon length
							);
						}
						
						
						ge->wakeAll();
						ge->clearAABB();
						
						
					}
					
					
				}
				
				for (bodInd = 0; bodInd < ge->bodies.size(); bodInd++) {
					curBody = &(ge->bodies[bodInd]);
					
					
					
					
					//$$$$$$$$$$$
					
					
					
					//////////////////////
					// APPLY FORCES
					//////////////////////
					
					ge->applyImpulses(curStepTime, bodInd);
					
					tempBTV = curBody->body->getCenterOfMassPosition();
					
					cellVal = singleton->gw->getCellAtCoords(
						tempBTV.getX(),
						tempBTV.getY(),
						tempBTV.getZ()
					);
					
					
					curBody->inWater = (cellVal == E_CD_WATER);
					curBody->isInside = (cellVal == E_CD_SOLID);
					
					// push out from underground
					
					if (curBody->isInside) {
						
						
						// ge->moveToPoint(tempBTV + btVector3(0,0,1.0), bodInd);
						// ge->applyImpulse(btVector3(0,0,1.0),false, bodInd);
						//curBody->lastVel = curBody->body->getLinearVelocity();
					}
					
					
					if (
						hasRig &&
						(curBody->boneId > -1) &&
						(ge->bodies.size() > 0)
						&& (bodInd > 0)
						&& animatedRig
						// && (curBody->boneId == E_BONE_C_SKULL)
						//false
					) {
						curOrgNode = curOrg->allNodes[curBody->boneId];
						
						if (curOrgNode == NULL) {
							
						}
						else {
							
							
							// if (bodInd == 0) {
							// 	//basePos = curOrgNode->orgTrans[1].getBTV();
							// }
							// else {
								
							// }
							
							
							// if (bodInd == 0) {
							// 	basePos = btVector3(0.0,0.0,0.0);
							// }
							// else {
							// 	basePos = 
							// 		curOrgNode->orgTrans[1].getBTV() * 
							// 		ge->bodies[0].body->getWorldTransform().getBasis() +
							// 		ge->bodies[0].body->getWorldTransform().getOrigin();
							// }
							
							
							
							
							ge->bodies[0].body->getWorldTransform().getOpenGLMatrix(myMat);
							myMatrix4 = Matrix4(myMat);
							
							if (curBody->isBall) {
								tempBTV = curOrgNode->orgTrans[2].getBTV();
							}
							else {
								tempBTV = curOrgNode->orgTrans[1].getBTV();
							}
							
							
								myVector4 = Vector4(
									tempBTV.getX(),
									tempBTV.getY(),
									tempBTV.getZ(),
									1.0f
								);
								resVector4 = myMatrix4*myVector4;
								basePos = btVector3(resVector4.x,resVector4.y,resVector4.z);
								
								
								
								ge->addAABBPoint(basePos);
								
								basePos += ge->skelOffset;// - btVector3(0.0f, 0.0f, BASE_ENT_HEIGHT*0.5f);
								
								// move limbs towards pose
								
								if (ge->weaponActive) {
									
									switch (curBody->boneId) {
										
										
										
										case E_BONE_L_METACARPALS:
										
											if (true) {//(curBody->isBall) {
												if (ge->rightHandTop) {
													difVec = ge->weaponVec0 - curBody->body->getCenterOfMassPosition();
												}
												else {
													difVec = ge->weaponVec0*0.8 + ge->weaponVec1*0.2 - curBody->body->getCenterOfMassPosition();
												}
											}
											else {
												difVec = btVector3(0.0f,0.0f,0.0f);
												
											}
										
											
										
											
										break;
										case E_BONE_R_METACARPALS:
										
											if (true) {//(curBody->isBall) {
												if (ge->rightHandTop) {
													difVec = ge->weaponVec0*0.8 + ge->weaponVec1*0.2 - curBody->body->getCenterOfMassPosition();
												}
												else {
													difVec = ge->weaponVec0 - curBody->body->getCenterOfMassPosition();
												}
											}
											else {
												difVec = btVector3(0.0f,0.0f,0.0f);
												
											}
										
											
										
											
										break;
										// case E_BONE_L_UPPERARM:
										// 	if (curBody->isBall) {
										// 		difVec = ge->leftVec*0.0002f;
										// 	}
										// 	else {
										// 		difVec = btVector3(0.0f,0.0f,0.0f);
										// 	}
											
										// break;
										// case E_BONE_R_UPPERARM:
										
										// 	if (curBody->isBall) {
										// 		difVec = ge->rightVec*0.0002f;
										// 	}
										// 	else {
										// 		difVec = btVector3(0.0f,0.0f,0.0f);
										// 	}
											
										// break;
										case E_BONE_L_LOWERARM:
										case E_BONE_R_LOWERARM:
										case E_BONE_L_UPPERARM:
										case E_BONE_R_UPPERARM:
											difVec = btVector3(0.0f,0.0f,0.0f);
										break;
										default:
											difVec = basePos - curBody->body->getCenterOfMassPosition();
										break;
									}
									
									
								}
								else {
									difVec = basePos - curBody->body->getCenterOfMassPosition();
								}
								
								
										
								totVec += difVec;
								
								ge->applyImpulse(
									difVec*curStepTime*curBody->mass*200.0f, // *MASS_PER_LIMB*2.0f*10.0f*curStepTime,
									false,
									bodInd
								);
							
							
							
							//
						}
						
						
					}
					
					if (ge->isGrabbedById > -1) {
						
						grabber = &(singleton->gw->gameObjects[ge->isGrabbedById]);
						
						doProc = true;
						
						if (curBody->boneId == E_BONE_WEAPON_END) {
							if (curBody->isBall) {
								basePos = grabber->weaponVec1;
							}
							else {
								basePos = grabber->weaponVec0*0.5f + grabber->weaponVec1*0.5f;
							}
						}
						else {
							if (curBody->boneId == E_BONE_WEAPON_BASE) {
								basePos = grabber->weaponVec0;
							}
							else {
								doProc = false;
							}
						}
						
						if (doProc) {
							difVec = basePos - curBody->body->getCenterOfMassPosition();
							
							// ge->applyImpulse(
							// 	difVec*curStepTime*curBody->mass*20.0f, // *MASS_PER_LIMB*2.0f*10.0f*curStepTime,
							// 	false,
							// 	bodInd
							// );
							
							ge->setLinVel(
								difVec*20.0f,
								bodInd
							);
						}
						
						
						
						
					}
					
					
					if (
						(singleton->selObjInd == ge->uid) &&
						singleton->markerFound &&
						singleton->isDraggingObject &&
						(singleton->draggingFromType == E_DT_WORLD_OBJECT)
					) {
						
						if (lastBodyPick == NULL) {
							
						}
						else {
							if (
								lastBodyPick->limbUID ==
								curBody->body->limbUID
							) {
								
								//totMass = ge->getTotalMass();
								
								//ge->applyAngularImpulse(btVector3(0,0,-0.02)/STEP_TIME_IN_SEC, false, curBody->body->limbUID);
								
								
							}
						}
						
						
						// ge->applyImpulse(
						// 	btVector3(
						// 		( singleton->worldMarker.getFX() - curBody->body->getCenterOfMassPosition().getX() ),
						// 		( singleton->worldMarker.getFY() - curBody->body->getCenterOfMassPosition().getY() ),
						// 		-(curBody->body->getCenterOfMassPosition().getZ() - (8.0f + singleton->worldMarker.getFZ()))
						// 	)*totMass*100.0f*curStepTime,
						// 	false,
						// 	0
						// );
						
						
						
						
						
					}
					
					
					
					for (m = 0; m < singleton->sphereStack.size(); m++) {
						tempBTV = curBody->body->getCenterOfMassPosition();
						tempBTV2 = singleton->sphereStack[m].position.getBTV();
						
						totForce = (
							1.0f-clampfZO(
								tempBTV.distance(tempBTV2)/(singleton->sphereStack[m].curRad*5.0f)	
							)
						)*5.0f; // * singleton->sphereStack[m].power;
						dirForce = tempBTV-tempBTV2;
						dirForce.normalize();
						dirForce = dirForce*totForce;
						
						dirForce.setZ(totForce);
						
						ge->applyImpulse(dirForce*curStepTime*20.0f*curBody->mass, false, bodInd);
					}
					
					
					//////////////////////
					// END APPLY FORCES
					//////////////////////
					
					
					
					
					
					nv0 = curBody->body->getLinearVelocity();
					nv1 = curBody->lastVel;
					
					if (nv0.isZero() || nv1.isZero()) {
						
					}
					else {
						nv0.normalize();
						nv1.normalize();
						
						
						if (
							(!(curBody->isInside)) &&
							(
								curBody->lastVel.length() > 0.5f
							) &&
							(
								(nv0.dot(nv1)) < 0.8f
							) &&
							(bodInd == 0) &&
							(ge->isGrabbedById < 0) 
							
						) {
							
							
							singleton->gw->fireEvent(
								ge->uid,
								EV_COLLISION,
								clampfZO( (curBody->lastVel.length()-0.5f)/16.0f )*
								(1.0f-clampfZO(ge->getCenterPointFIV(bodInd)->distance(singleton->cameraGetPosNoShake())/(50.0f)))
							);
						}
					}
					
					
					
					
					curBody->lastVel = curBody->body->getLinearVelocity();
					
					
					
					if (ge->entType == E_ENTTYPE_BULLET) {
						if (
							(!(curBody->isFalling)) && 
							(curBody->body->getLinearVelocity().length() < 0.5)
						) {
							singleton->explodeBullet(ge);
						}
					}
					
					
					
					
					//$$$$$$$$$$$
					
				}
				
				ge->zeroZ = false;
				
				//ge->skelOffset *= 0.99;
				
				if (ge->baseContact()&&hasRig&&animatedRig) {
					
					
					
					ge->skelOffset = btVector3(
						0.0f,
						0.0f,
						
						
							
						(
							(ge->getCenterPoint(0).getZ()-BASE_ENT_HEIGHT*0.5f) -
							ge->aabbMin.getZ()	
						)
							
						
							
					);
					
					if (singleton->editPose) {
						
					}
					else {
						ge->skelOffset += btVector3(
							0.0,
							0.0,
							-1.0f	
						);
					}
					
										
					// if (totVec.getZ() < 0.0f) {
						
					// 	ge->skelOffset += btVector3(
					// 			0.0f,
					// 			0.0f,
					// 			-totVec.getZ()*0.3f	
					// 	);
					// }
					
					
					
					
					// if ( singleton->feetContact(ge) ) {
					// 	ge->skelOffset += btVector3(
					// 			0.0f,
					// 			0.0f,
					// 			0.003f
					// 	);
					// }
					// else {
					// 	ge->skelOffset += btVector3(
					// 			0.0f,
					// 			0.0f,
					// 			-0.002f	
					// 	);
					// }
					
					
				}
				else {
					ge->skelOffset *= 0.99;
				}
				
			}
			
			
		}
		
		singleton->explodeStack.clear();
		
	}
void GamePhysics::updateAll ()
                         {
		
		while (singleton->totTimePassedPhysics > STEP_TIME_IN_MICRO_SEC) {
			totTime += STEP_TIME_IN_SEC;
			collideWithWorld(STEP_TIME_IN_SEC); //STEP_TIME_IN_MICRO_SEC/400000.0f
			example->stepSimulation(STEP_TIME_IN_SEC); //STEP_TIME_IN_MICRO_SEC/400000.0f
			singleton->totTimePassedPhysics -= STEP_TIME_IN_MICRO_SEC;
		}
		
		flushImpulses();
	}
GamePhysics::~ GamePhysics ()
                       {
		example->exitPhysics();
		delete example;
	}
#undef LZZ_INLINE
 
