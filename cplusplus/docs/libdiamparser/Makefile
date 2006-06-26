ID=1
SPECVERSION=00
SPECFILE=draft-ohba-aaa-diameter-cxxapi-$(SPECVERSION).txt
TOCFILE=cxxapi.toc
$(SPECFILE): cxxapi.nroff
ifdef ID
	rm -f $(TOCFILE)
	(groff -ms -Tascii $< > aaa.txt) >& $(TOCFILE)
	(groff -ms -Tascii $< > aaa.txt) >& aaa.toc
	mv aaa.toc $(TOCFILE)
	(groff -ms -Tascii $< > aaa.txt) > /dev/null 2>&1  
	sed -e 's/\[Page [0-9].*\]/&/' aaa.txt > $@
	rm aaa.txt
else
	groff -mm -Tascii $< > $@
endif

clean:
	rm -f $(SPECFILE) $(TOCFILE) *~
