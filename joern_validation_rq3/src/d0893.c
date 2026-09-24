bool ContainerChunk::removeValue( XMP_Uns32	id )
{
	valueMap* cm = &this->childmap;
	valueMapIter iter = cm->find( id );

	if( iter == cm->end() )
		return false;  //not found

	ValueChunk* propChunk = iter->second;

	chunkVect* cv = &this->children;
	chunkVectIter cvIter;
	for (cvIter = cv->begin(); cvIter != cv->end(); ++cvIter )
	{
		if ( (*cvIter)->id == id )
			break; // found!
	}
	XMP_Validate( cvIter != cv->end(), "property not found in children vector", kXMPErr_InternalFailure );
	cv->erase( cvIter );

	cm->erase( iter );

	delete propChunk;
	return true; // found and removed
}
