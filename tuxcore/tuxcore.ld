SECTIONS {
    __start_.version = .;
    .version :
    {
	    *(version.1)
	    *(version.2)
	    *(version.3)
    }
    __stop_.version = .;
}

