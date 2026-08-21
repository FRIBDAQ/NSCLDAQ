#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2014, 2026
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#	     FRIB
#	     Michigan State University
#	     East Lansing, MI 48824-1321

'''
    @file LogBookUIUtilities.py
    @brief Reusable GUI Utilities for the lg_xxx.py programs.
    @author Ron Fox
    @note See individual class and method docstrings for more information.
'''


from nscldaq.LogBook import LogBook
from nscldaq.editablelist6 import ListToListEditor

from PyQt6.QtWidgets import QListWidget

class ShiftMemberEditor(ListToListEditor):
    '''
        Provides an editor of shift membership
        that is based on the list to list editor. The differences
        are that elements of each list are derived from LogBook.Person lists
        rather than textual lists
        
        We'll do this by holding a map of tuples -> Person objects.
        where the tuples are used to derive the contents of the 
        listboxes and discover the people in each listbox.
        
        Attributes:
          nonMembers - list of LogBook.Person that are not in the shift
          members    - list of LogBook.Person that are in the shift
          
    '''
    def __init__(self, *args):
        super().__init__(*args)
    
        # These are hashes of LogBook.Person
        # Keyed by the (lastname, firstname, salutation) tuple.
    
        self._members = {}    
        self._nonmembers = {}
        
    # Attributes:
    
    def members(self) -> list[LogBook.Person]:
        self._reconstructDicts()
        return list(self._members.values())     # Want an actual list not a view.
    def setMembers(self, members : list[LogBook.Person]) -> None:
        member_list = self.selectedbox().listbox()   # QListWidget 
        self._members = self._setHashAndListBox(member_list, members)
        
    
    def nonMembers(self) -> list[LogBook.Person]:
        self._reconstructDicts()
        return list(self._nonmembers.values())
    
    def setNonMembers(self, nonmembers : list[LogBook.Person]):
        nonmember_list = self.sourcebox()
        self._nonmembers = self._setHashAndListBox(nonmember_list, nonmembers)
        
        
    def _setHashAndListBox(
        self, 
        listbox : QListWidget, people : list[LogBook.Person]) -> dict[(str, str, str), LogBook.Person]:
        #  Given a listbox and a list of people, construct the dict hash of the people and fill the
        # listbox with salutation firstname, lastname.
        
        listbox.clear()
        result : dict[(str,str,str), LogBook.Person] = {}
        
        for person in people:
            lastname = person.lastname
            firstname = person.firstname
            salutation = person.salutation
            result[(lastname, firstname, salutation)] = person
            
            item = f'{salutation} {firstname} {lastname}'
            listbox.addItem(item)
        
        return result
        
    def _reconstructDicts(self) -> None:
        # Since we have no signals to keep the dicts updated, we
        # need to reconstruct them when asked for the contents:
        
        memberstuples = self._listboxToTuples(self.selectedbox().listbox())
        nonmembertuples = self._listboxToTuples(self.sourcebox())
        
        members = {}
        nonmembers = {}
        
        # Build the members hash:
        
        for member_tuple in memberstuples:
            if member_tuple in self._members:
                members[member_tuple] = self._members[member_tuple]
            else:
                members[member_tuple] = self._nonmembers[member_tuple]
    
        # build the nonmembers hash:
        
        for nonmember_tuple in nonmembertuples:
            if nonmember_tuple in self._members:
                nonmembers[nonmember_tuple] = self._members[nonmember_tuple]
            else:
                nonmembers[nonmember_tuple] = self._nonmembers[nonmember_tuple]
            
        # NOw set the attribute data:
        
        self._members = members
        self._nonmembers = nonmembers
        
    def _listboxToTuples(self, listbox : QListWidget) -> list[tuple[str, str, str]]:
        # Turn the contents of a listbox widget into lastname, salutation tuples:
        
        result = []
        for row in range(listbox.count()):
            rowtext = listbox.item(row).text()
            (salutation, firstname, lastname) = rowtext.split(' ')
            result.append((lastname, firstname, salutation))
        return result

#   Test code:

if __name__  == "__main__":
    import sys
    from PyQt6.QtWidgets import QApplication
    from PyQt6.QtCore    import QTimer
    from nscldaq.LogBook import logbookadmin, LogBook
    import tempfile
    import pathlib
    
    def listmembers(w : ShiftMemberEditor) -> None:
        members = w.members()
        nonmembers = w.nonMembers()
        
        print ("Members:")
        for member in members:
            print(member.lastname)
        
        print('Nonmembers:')
        for person in nonmembers:
            print(person.lastname)
            
            
    
    app = QApplication(sys.argv)
    w   = ShiftMemberEditor()
    w.show()
    
    # We need the file to no longer exist when we create the logbook:

    name = tempfile.NamedTemporaryFile().name
    
    lb = logbookadmin.createLogBook(name, '0400x', 'ron fox', 'nothing', False)
    
    # Make some people and load them all into the
    # nonmembers list:
    
    people = []
    people.append(lb.add_person('Fox', 'Ron', "Mr."))
    people.append(lb.add_person('Cerizza', 'Giordano', "Dr."))
    people.append(lb.add_person('Chang', 'JinHee', 'Dr.'))
    
    w.setNonMembers(people)
    
    # I _think_ this is safe:
    
    pathlib.Path(name).unlink()      # Probalby held untile the database is deleted.
    
    # Periodically output the members/nonmembers.
    
    timer = QTimer()
    timer.setSingleShot(False)      # Repeating.
    timer.timeout.connect(lambda : listmembers(w))
    timer.start(2000)
    
    sys.exit(app.exec())
    