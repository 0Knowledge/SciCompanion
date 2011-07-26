#include "stdafx.h"

#include "ResourceRecency.h"

int _GetLookupKey(const IResourceIdentifier *pData)
{
    // These should always be already-added resources, so they should never be -1
    ASSERT(pData->GetNumber() != -1);
    ASSERT(pData->GetPackageHint() != -1);
    ASSERT(pData->GetType() < NumResourceTypes);
    return pData->GetNumber() * 256 * 256 + pData->GetPackageHint();
}

template<typename _T, typename _K>
bool contains(const _T &container, const _K &key)
{
    return container.find(key) != container.end();
}

void ResourceRecency::AddResourceToRecency(const ResourceBlob *pData, BOOL fAddToEnd)
{
    _idJustAdded = -1;
    int iKey = _GetLookupKey(pData);

    ResourceIdArray *pidList;
    RecencyMap::iterator found = _resourceRecency[pData->GetType()].find(iKey);
    if (found != _resourceRecency[pData->GetType()].end())
    {
        pidList = found->second;
#ifdef DEBUG
        // Now that we use a checksum instead of an id, this is not a valid ASSERT...
        // Assert that a resource of this id doesn't not already exist in here.
        /*for (INT_PTR i = 0; i <= pidList->GetUpperBound(); i++)
        {
            ASSERT(pData->GetId() != pidList->GetAt(i));
        }*/
#endif
    }
    else
    {
        // A new one.
        pidList = new ResourceIdArray;
        _resourceRecency[pData->GetType()][iKey] = pidList;
    }
    // Add this resource.
    if (fAddToEnd)
    {
        // This is called when we're enumerating.
        pidList->push_back(pData->GetId());
    }
    else
    {
        // This is called when the user adds a resource.
        _idJustAdded = pData->GetId();
        pidList->insert(pidList->begin(), pData->GetId());
    }
}

void ResourceRecency::DeleteResourceFromRecency(const ResourceBlob *pData)
{
    int iKey = _GetLookupKey(pData);

    RecencyMap::iterator found = _resourceRecency[pData->GetType()].find(iKey);
    if (found != _resourceRecency[pData->GetType()].end())
    {
        ResourceIdArray *pidList = found->second;
        // Find this item's id.
        for (ResourceIdArray::iterator it = pidList->begin(); it != pidList->end(); ++it)
        {
            if (*it == pData->GetId())
            {
                // Found it. Remove it.
                pidList->erase(it);
                break;
            }
        }
    }
}

//
// Is this the most recent resource of this type.
//
bool ResourceRecency::IsResourceMostRecent(const IResourceIdentifier *pData)
{
    bool fRet;
    if (pData->GetNumber() == -1)
    {
        // The resource is not saved - so consider it the "most recent" version of itself.
        fRet = true;
    }
    else
    {
        fRet = false;
        int iKey = _GetLookupKey(pData);

        RecencyMap::iterator found = _resourceRecency[pData->GetType()].find(iKey);
        if (found != _resourceRecency[pData->GetType()].end())
        {
            ResourceIdArray *pidList = found->second;
            // (should always have at least one element)
            fRet = !pidList->empty() && (*pidList->begin() == pData->GetId());
        }
    }
    return fRet;
}

bool ResourceRecency::WasResourceJustAdded(const ResourceBlob *pData)
{
    return (pData->GetId() == _idJustAdded);
}

void ResourceRecency::ClearResourceType(int iType)
{
    ASSERT(iType < NumResourceTypes);

    RecencyMap &map = _resourceRecency[iType];
    // Before removing all, we must de-allocate each array we created.
    for_each(map.begin(), map.end(), delete_map_value());
    map.clear();
}

void ResourceRecency::ClearAllResourceTypes()
{
    for (int i = 0; i < NumResourceTypes; i++)
    {
        ClearResourceType(i);
    }
}
