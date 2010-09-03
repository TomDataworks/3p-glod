/******************************************************************************
 * Copyright 2004 David Luebke, Brenden Schubert                              *
 *                University of Virginia                                      *
 ******************************************************************************
 * This file is distributed as part of the VDSlib library, and, as such,      *
 * falls under the terms of the VDSlib public license. VDSlib is distributed  *
 * without any warranty, implied or otherwise. See the VDSlib license for     *
 * more details.                                                              *
 *                                                                            *
 * You should have recieved a copy of the VDSlib Open-Source License with     *
 * this copy of VDSlib; if not, please visit the VDSlib web page,             *
 * http://vdslib.virginia.edu/license for more information.                   *
 ******************************************************************************/
catch(ZThreads::ztMemoryAllocationFailure t) 
{ 
cout << "\nZThreads Exception Caught!" << endl; 
t;
}

catch(ZThreads::ztThreadCreationFailure t) 
{ 
cout << "\nZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztSyncObjectCreationFailure t) 
{ 
cout << "\nZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztBadParametersError t) 
{ 
cout << "\nZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztDestructBusySyncObjectError t) 
{ 
cout << "\nztDestructBusySyncObjectError ZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztSetPreviouslySetFlagError t) 
{ 
cout << "\nztSetPreviouslySetFlagError ZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztLockHeldLockError t) 
{ 
//L.Lock();
cout << "\nZThreads ztLockHeldLockError Exception Caught!" << endl;  
t;
//L.Unlock();
}

catch(ZThreads::ztUnlockUnheldLockError t) 
{ 
cout << "\nztUnlockUnheldLockError ZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztIncrementCounterOverflowError t) 
{ 
cout << "\nztIncrementCounterOverflowErrorZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztPassInvalidThreadBarrierError t) 
{ 
cout << "\nztPassInvalidThreadBarrierError ZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztResetBusySyncObjectError t) 
{ 
cout << "\nztResetBusySyncObjectError ZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztErrorException t) 
{ 
cout << "\nZThreads Exception Caught!" << endl;  
t;
}

catch(ZThreads::ztFailureException t) 
{ 
cout << "\nZThreads Exception Caught!" << endl;  
t;
}
