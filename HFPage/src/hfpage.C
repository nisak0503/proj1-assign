#include <iostream>
#include <stdlib.h>
#include <memory.h>


#include "heapfile.h"
#include "buf.h"
#include "db.h"
#include "hfpage.h"




// **********************************************************
// page class constructor

void HFPage::init(PageId pageNo)
{
	cerr << "init @kasin"<<endl;
  // fill in the body
	//setting data members to reasonable default
	nextPage = INVALID_PAGE;
	prevPage = INVALID_PAGE;
	slotCnt = 0;
	slot[0].length = EMPTY_SLOT;
	curPage = pageNo; 
	usedPtr = MAX_SPACE-DPFIXED; //usedPtr is the offset of first used byte in data[]
	freeSpace = MAX_SPACE - DPFIXED + sizeof(slot_t); // number of bytes free in data[]
	memset(data, '\0', sizeof(data)); 
	
}

// **********************************************************
// dump page utlity
void HFPage::dumpPage()
{
    int i;

    cout << "dumpPage, this: " << this << endl;
    cout << "curPage= " << curPage << ", nextPage=" << nextPage << endl;
    cout << "usedPtr=" << usedPtr << ",  freeSpace=" << freeSpace
         << ", slotCnt=" << slotCnt << endl;
   
    for (i=0; i < slotCnt; i++) {
        cout << "slot["<< i <<"].offset=" << slot[i].offset
             << ", slot["<< i << "].length=" << slot[i].length << endl;
    }
}

// **********************************************************
PageId HFPage::getPrevPage()
{
	cerr << "getPrevPage @kasin" << endl;
    // fill in the body
//	if (prevPage != INVALID_PAGE)
		return prevPage;
//	else
//		return prevPage;
//    	return 0;
}

// **********************************************************
void HFPage::setPrevPage(PageId pageNo)
{
	cerr << "setPrevPage @kasin"<< endl;
    // fill in the body
	prevPage = pageNo;
}

// **********************************************************
void HFPage::setNextPage(PageId pageNo)
{
	cerr << "setNextPage @kasin" << endl;
  // fill in the body
	nextPage = pageNo;
}

// **********************************************************
PageId HFPage::getNextPage()
{
	cerr << "getNextPage @kasin" << endl;
    // fill in the body
//	if (nextPage != INVALID_PAGE)	
		return nextPage;
//	else
//    	return 0;
}



// **********************************************************
// Add a new record to the page. Returns OK if everything went OK
// otherwise, returns DONE if sufficient space does not exist
// RID of the new record is returned via rid parameter.
Status HFPage::insertRecord(char* recPtr, int recLen, RID& rid)
{
	cerr << "insertRecord @kasin"<<recLen <<"~"<<recPtr[0]<<"~"<<endl;
	printf("point = %c\n", recPtr[0]);
    // fill in the body
	// when we judge we have space for the record
	// char pointer points to the record with recLen 
	// data has space, slot directory has space
	//@@@@@@short slotId = hasSpace(recLen, slot, freeSpace, slotCnt);
	/*implement hasSpace in this function*/
	short slotId = -1;	
	if(freeSpace < recLen) return DONE;
	for(short s = 0; s < slotCnt; ++s)
	{
		if(slot[s].length == EMPTY_SLOT)
		{
			slotId = s;
			break;
		}
	}
	if(slotId == -1){  //no pre-allocated empty slot, has to new one 
		if(freeSpace >= recLen + sizeof(slot_t))
		{
			slotId = slotCnt;
			slotCnt++;
			freeSpace -= sizeof(slot_t);
		}else{
			cerr << "@kasin has no Space for slot and data!!";
			return DONE;
		}
	}
	//else do thing because slotId has value
	
	//slot
	//already coped with freeSpace update
	usedPtr -= recLen;
	slot[slotId].offset = usedPtr;
	slot[slotId].length = recLen;
	//data
	memcpy(data+slot[slotId].offset, recPtr, recLen);
	freeSpace -= recLen;
	//rid
	rid.pageNo = curPage;
	rid.slotNo = slotId;	
    return OK;
}

// **********************************************************
// Delete a record from a page. Returns OK if everything went okay.
// Compacts remaining records but leaves a hole in the slot array.
// Use memmove() rather than memcpy() as space may overlap.
Status HFPage::deleteRecord(const RID& rid)
{
	cerr << "deleteRecord @kasin"<<endl;
    // fill in the body
	if(rid.pageNo != curPage) return FAIL;
	short slotId = rid.slotNo;
	if(slotId < 0) return FAIL;
	if(slotId >= slotCnt) return FAIL;
	short curOffset = slot[slotId].offset;
	short curLength = slot[slotId].length;
	//              XXXXXXXXXXX   YYYYYYYYYYYYYYY
    //              |
	//              |  XXXXXXXXXXXYYYYYYYYYYYYYYY            
	//     data+usedPtr|
	//                 |
	//			data+usedPtr+length
	
	memmove(data+usedPtr+curLength, data+usedPtr, curLength);
	for(int s = 0; s < slotCnt; ++s)
	{
		if(slot[s].offset < curOffset)
			slot[s].offset += curLength;
	}
	usedPtr += curLength;
	freeSpace += curLength;
	slot[slotId].length = EMPTY_SLOT;
	slot[slotId].offset = -1;
	while((slotId == slotCnt-1)&&(slot[slotId].length == EMPTY_SLOT))
	{
		slotCnt--;
		slotId--;
		freeSpace += sizeof(slot_t);
	}
    return OK;
}

// **********************************************************
// returns RID of first record on page
Status HFPage::firstRecord(RID& firstRid)
{
	cerr << "firstRecord @kasin"<<endl;
    // fill in the body

	short slotId = -1;
	for(short s = 0; s < slotCnt; ++s)
	{
		if(slot[s].length == EMPTY_SLOT) continue;
		if(slotId == -1) slotId = s;
		else
			slotId = slot[slotId].offset < slot[s].offset ? s : slotId;
	}
/*	short aimOffset = usedPtr;
	for (short s = 0; s < slotCnt; ++s)
		if(slot[s].offset == aimOffset)
		{
			slotId = s;
			break;
		}
*/
	if (slotId == -1) 
	{
		cerr << "there is no first Record! by kasin"<<endl;
	//	while(true);
		return DONE;
	}
	firstRid.pageNo = curPage;
	firstRid.slotNo = slotId;
    return OK;
}

// **********************************************************
// returns RID of next record on the page
// returns DONE if no more records exist on the page; otherwise OK
Status HFPage::nextRecord (RID curRid, RID& nextRid)
{
	cerr << "nextRecord @kasin"<<endl;
    // fill in the body
	PageId nextPageNo = curRid.pageNo;
	if (nextPageNo != curPage) return DONE;
	//cal slotNo
	short nowSlotId = curRid.slotNo;
	if(nowSlotId  < 0) return FAIL;
	if(nowSlotId >= slotCnt) return FAIL;
//	short nextOffset = slot[nowSlotId].offset  slot[nowSlotId].length;
	for (short s = 0; s < slotCnt; ++s)
	{
		if(slot[s].length == EMPTY_SLOT) continue;
		if(slot[s].offset + slot[s].length == slot[nowSlotId].offset)
		{
			nextRid.pageNo = nextPageNo;
			nextRid.slotNo = s;
			return OK;
		}
	}
	return DONE;
    return OK;
}

// **********************************************************
// returns length and copies out record with RID rid
Status HFPage::getRecord(RID rid, char* recPtr, int& recLen)
{
	cerr << "getRecord @kasin"<<endl;
    // fill in the body
	if(rid.pageNo != curPage) return FAIL;
	short slotId = rid.slotNo;
	if(slotId >= slotCnt) return FAIL;
	recLen = slot[slotId].length;
	memcpy(recPtr, data + slot[slotId].offset, slot[slotId].length);
    return OK;
}

// **********************************************************
// returns length and pointer to record with RID rid.  The difference
// between this and getRecord is that getRecord copies out the record
// into recPtr, while this function returns a pointer to the record
// in recPtr.
Status HFPage::returnRecord(RID rid, char*& recPtr, int& recLen)
{
	cerr << "returnRecord @kasin"<<endl;
    // fill in the body
	if(rid.pageNo != curPage) return FAIL;
	short slotId = rid.slotNo;
	if(slotId < 0) return FAIL;
	if(slotId >= slotCnt) return FAIL;
	if(slot[slotId].length == EMPTY_SLOT) return FAIL;
	
	recLen = slot[slotId].length;
	cerr << "len = "<< recLen <<" slot.offset = "<<slot[slotId].offset<<endl;
	// you set the caller's recPtr to point directly to the record on the page.
	recPtr = &data[slot[slotId].offset];
    return OK;
}

// **********************************************************
// Returns the amount of available space on the heap file page
int HFPage::available_space(void)
{
	cerr << "available_space @kasin"<<endl;
    // fill in the body
	//since freeSpace does not count slot[0]
	short slotId = -1;
	for (short s = 0; s < slotCnt; ++s)
	{
		if(slot[s].length == EMPTY_SLOT)
		{
			slotId = s;
			break;
		}
	}
	//there is a pre-allocated empty slot that can be used
	//since slot[0] cannot contain data, dont have to freeSpace + sizeof(slot_t) even if slot[0].length == EMPTY_SLOT  
	if(slotId != -1) 
	{
		cerr << "has pre-allocated empty space @kasin "<< freeSpace<<endl;
		return freeSpace; 
	}	
	else //have to new a slot
	{	
		cerr << "have to new a slot @kasin  "<<freeSpace-sizeof(slot_t)<<endl;
		return freeSpace - sizeof(slot_t);
	}    
	return 0;
}

// **********************************************************
// Returns 1 if the HFPage is empty, and 0 otherwise.
// It scans the slot directory looking for a non-empty slot.
bool HFPage::empty(void)
{
	cerr << "empty @kasin"<<endl;
    // fill in the body
	for(short s = 0; s < slotCnt; ++s)
	{
		if(slot[s].length != EMPTY_SLOT) return false;
	}
    return true;
}



