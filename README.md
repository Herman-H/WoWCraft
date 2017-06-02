# WoWCraft
A program to view and edit game data that is used by an emulated WoW server, implemented by the open source project CMaNGOS. 

Note that this is a transition from project "wow_db_editor". "wow_db_editor" is currently kept as a reference. The problem with "wow_db_editor" is that since the goal is for it to be usable, it's messy to change or add new things to the project and since some things are very inefficiently implemented, load times are too high (not counting the compile time). WoWCraft is a fresh start, salvaging the best ideas that came from developing "wow_db_editor".

Since the major problem was slow load times, the focus from the start will here be to have a very efficient and flexible resource managing system.

### Specifications on the resource management system
#### Loading
- All GUI elements that uses some persistent storage resource, should be possible to construct instantly, without waiting for the resource to load. We call these GUI elements views.
- A view should be able to view the loading process of the resources it is dependent on, and display this progress in some appropriate manner.
- It should be possible to interact with the view, in case there is an error with the resource it is dependent on and if there is an error, it should also be displayed in some appropriate way. For example, if a file is missing from the configured directory from where to load the file, you can now click on the view and be guided on how to resolve this error, in this case locate the file.
- A "resource directed acyclic graph", which will be at the heart of the entire resource system. This is an object which takes as input a description of ALL files or database tables that the program needs and all derivative objects that are used in views and all dependencies that exists between all these resources. Then, through this object it is now possible to initialize resources in some order, concurrently, for maximum efficiency. For example, it will be assumed that the system can only provide 1 disk access at a time, so diskaccesses could be scheduled to run after each other on one single thread. But once a file has been loaded, it can now be used by derivative resources depending on it and thus we can start new threads to initialize those resources while we are still loading new files. Some derivative resources need more time to initialize, so these could be scheduled to load first, for example.
#### Modifications and storage
- When modifying data, the change should be visible instantly and the change should propagate, visibly, to all views that should be affected. The change should not be written to persistent storage BUT a backup file describing editing deltas, forward and backward, could be saved. The reason for this is simply to semantically seperate view updates from persistent storage updates, because otherwise, the application would freeze for some noticeable instant and this would not FEEL efficient.
- All changes to data can be saved to persistent storage by pressing a button, something like "Save Changes". For any persistent storage writes, we should use a new thread for this work.

### Current implementation
Right now I have a simple dbc-file to QAbstractItemModel implementation. Compare the complete chain done in "wow_db_editor" where I don't even use item models. Now I can describe a dbc file with a minimal description (check in dbc/dbc_files.h), use a projection (dbc/dbc_projection.h) of this file onto a table (check dbc/dbc_record.h and dbc/dbc_table.h) which can be adapted to a QAbstractItemModel (check widgets/dbc_item_model.h) which can then be viewed by multiple different views. In this chain the only unique information is the file description, the projection description and what view to use. If we study the old way to implement "views" in "wow_db_editor" (check wow_db_editor/dbcwidgets.h and wow_db_editor/dbcwidgets.cpp) a lot of descriptions in there really are redundant which is why those files are huge. That problem is now solved here, though I still need to actually add the functionality used there, mainly "selectIndex()" and "setIndexFromId()" and it doesn't meet the bulk of the above specifications.
Check widgets/simple_dbc_combobox.h to see how easily a dbc resource is initialized and check in MainWinow how easily it is used.

### Using the current implementation
The program will automatically create a "dbc" folder where it runs. You should there put the files contained in WoWCraft/dbc/dbc/. When starting the program it should now load properly.
