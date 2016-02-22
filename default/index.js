'use strict';

const fs = require('fs-extra');
const path = require('path');
const silkedit = require('silkedit');
const DialogUtils = require('./lib/dialog_utils');
const InputDialog = require('./lib/input_dialog');

// Replace oldexp (string or regexp) in a file content with newStr
function replaceFileContent(file, oldexp, newStr, cb) {
	fs.readFile(file, 'utf-8', (err, data) => {
		if (err) return cb(err)
					
		const result = data.replace(oldexp, newStr)
		fs.writeFile(file, result, 'utf-8', (err) => {
			if (err) return cb(err)
			cb()
		})
	})
}

function move_cursor(operation, repeat) {
  const editView = silkedit.App.activeTextEditView();
  if (editView != null) {
    repeat = typeof repeat === 'number' ? repeat : 1;
    const cursor = editView.textCursor();
    cursor.movePosition(operation, silkedit.TextCursor.MoveMode.MoveAnchor, repeat);
    editView.setTextCursor(cursor);
  }
}

const textEditFocusCond = {
  keyValue: () => {
    return silkedit.App.focusWidget() instanceof silkedit.TextEditView;
  }
}

const consoleVisibleCond = {
  keyValue: () => {
    const win = silkedit.App.activeWindow();
    if (win) {
      const view = win.console();
      if (view) {
        return view.visible;
      }
    }
    return false;
  }
}

module.exports = {
	activate: () => {
    silkedit.Condition.add("console_visible", consoleVisibleCond);
    silkedit.Condition.add("text_edit_focus", textEditFocusCond);
	},
	
	deactivate: () => {
	  silkedit.Condition.remove("console_visible");
    silkedit.Condition.remove("text_edit_focus");
	},

	commands: {
		"new_file": () => {
			const tabView = silkedit.App.activeTabView()
			if (tabView != null) {
				tabView.addNew();
			} else {
			  console.log('active tab view is null');
			}
		},
		"open": () => {
			const paths = DialogUtils.showFileAndDirectoryDialog('Open');
			paths.forEach(function(path) {
				fs.stat(path, (err, stats) => {
				  if (err) {
				    console.error(err);
				    return;
				  }
				  
				  if (stats.isFile()) {
				    silkedit.DocumentManager.open(path);
				  } else if (stats.isDirectory()) {
				    silkedit.ProjectManager.open(path);
				  } else {
				    console.warn(`${path} is neither file nor directory`);
				  }
				});
			});
		},
		"open_file": () => {
			const paths = DialogUtils.showFileDialog('Open File')
			paths.forEach(function(path) {
				silkedit.DocumentManager.open(path)
			})
		},
		"open_folder": () => {
			const paths = DialogUtils.showDirectoryDialog('Open Folder')
			paths.forEach(function(path) {
				silkedit.ProjectManager.open(path)
			});
		},
		"save": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.save()
			}
		},
		"save_as": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.saveAs()
			}
		},
		"save_all": () => {
			const tabViewGroup = silkedit.App.activeTabViewGroup();
			if (tabViewGroup != null) {
				tabViewGroup.tabViews.forEach((tabView) => {
				  for (let i = 0; i < tabView.count; i++) {
				    const widget = tabView.widget(i);
				    if (widget instanceof silkedit.TextEditView) {
				      widget.save();
				    }
				  }
				});
			}
		},
		"close_all_tabs": () => {
			const tabView = silkedit.App.activeTabView()
			if (tabView != null) {
				tabView.closeAllTabs();
			}
		},
		"close_other_tabs": () => {
			const tabView = silkedit.App.activeTabView()
			if (tabView != null) {
				tabView.closeOtherTabs();
			}
		},
		"close_tab": () => {
			const tabView = silkedit.App.activeTabView()
			if (tabView != null) {
				if (tabView.count > 0) {
					tabView.closeActiveTab();
				} else {
					const win = silkedit.App.activeWindow()
					if (win != null) {
						win.close()
					}
				}
			}
		},
		"undo": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.undo()
			}
		},
		"redo": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.redo()
			}
		},
		"cut": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.cut()
			}
		},
		"copy": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.copy()
			}
		},
		"paste": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.paste()
			}
		},
		"select_all": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.selectAll()
			}
		},
		"complete": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.performCompletion()
			}
		},
		"delete": (args) => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1
				editView.deleteChar(repeat)
			}
		},
		"delete_backward": (args) => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1
				editView.deleteChar(-1 * repeat)
			}
		},
		"move_cursor_up": (args) => {
      const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1;
      move_cursor(silkedit.TextCursor.MoveOperation.Up, repeat);
		},
		"move_cursor_down": (args) => {
      const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1;
      move_cursor(silkedit.TextCursor.MoveOperation.Down, repeat);
		},
		"move_cursor_left": (args) => {
      const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1;
      move_cursor(silkedit.TextCursor.MoveOperation.Left, repeat);
		},
		"move_cursor_right": (args) => {
      const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1;
      move_cursor(silkedit.TextCursor.MoveOperation.Right, repeat);
		},
		"move_cursor_start_of_line": (args) => {
      const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1;
      move_cursor(silkedit.TextCursor.MoveOperation.StartOfBlock, repeat);
		},
    "move_cursor_end_of_line": (args) => {
      const repeat = 'repeat' in args ? Number.parseInt(args.repeat) : 1;
      move_cursor(silkedit.TextCursor.MoveOperation.EndOfBlock, repeat);
		},
		"find_and_replace": () => {
			const win = silkedit.App.activeWindow()
			if (win != null) {
				win.openFindAndReplacePanel()
			}
		},
		"split_horizontally": () => {
			const tabViewGroup = silkedit.App.activeTabViewGroup()
			if (tabViewGroup != null) {
				tabViewGroup.splitHorizontally();
			}
		},
		"split_vertically": () => {
			const tabViewGroup = silkedit.App.activeTabViewGroup()
			if (tabViewGroup != null) {
				tabViewGroup.splitVertically();
			}
		},
		"show_scope": () => {
			const win = silkedit.App.activeWindow()
			const editView = silkedit.App.activeTextEditView()
			if (win != null && editView != null) {
				win.statusBar().showMessage(editView.scopeName());
			}
		},
		"show_scope_tree": () => {
			const win = silkedit.App.activeWindow()
			const editView = silkedit.App.activeTextEditView()
			if (win != null && editView != null) {
				silkedit.alert(editView.scopeTree());
			}
		},
		"new_package": () => {
      var validate;
      silkedit.PackageManager._ensureRootPackageJson();
      fs.readFile(silkedit.Constants.userRootPackageJsonPath, 'utf-8', (err, data) => {
        let rootObj = {};
        if (err) {
          validate = null
        } else {
          rootObj = JSON.parse(data);
          try {
            let pkgnameSet = new Set();
            if ('dependencies' in rootObj) {
              const packages = Object.keys(rootObj['dependencies']);
              pkgnameSet = new Set(packages);
            }
            validate = (pkgName, callback) => {
              callback(!pkgnameSet.has(pkgName))
            }
          } catch(err) {
            console.error(err)
            validate = null
          }
        }

        const dialog = new InputDialog(silkedit.tr("enter_new_package_name", "Enter new package name"), "my_package", validate);
        const pkgName = dialog.show();
  			// copy hello example package to a new package directory
  			if (pkgName == null || pkgName.length == 0) return
				
  		  const pkgPath = path.join(silkedit.Constants.userPackagesNodeModulesPath, pkgName)
  		  console.log(pkgPath);
   			fs.copy(__dirname + "/resources/hello", pkgPath, (err) => {
   				if (err) throw err

   			  // replace <name> with the new package name
   			  // open the package dir as project
   			  const newPkgJsonPath = pkgPath + "/package.json"
   			  replaceFileContent(newPkgJsonPath, /<name>/g, path.basename(pkgPath), (err) => {
   				  if (err) throw err
   				  replaceFileContent(pkgPath + "/menu.yml", /<name>/g, path.basename(pkgPath), (err) => {
   					  if (err) throw err
   					  replaceFileContent(pkgPath + "/index.js", /<name>/g, path.basename(pkgPath), (err) => {
     					  if (err) throw err
     					  replaceFileContent(pkgPath + "/README.md", /<name>/g, path.basename(pkgPath), (err) => {
      					  if (err) throw err
     					    // append new package's package.json content to packages.json
     					    fs.readFile(newPkgJsonPath, (err, data) => {
    					      if (err) throw err

                    const newPkgJsonData = JSON.parse(data);
                    const url = newPkgJsonData.repository + "#" + newPkgJsonData.version;
                    if ('dependencies' in rootObj) {
                      rootObj['dependencies'][pkgName] = url;
                    } else {
                      const obj = {};
                      obj[pkgName] = url;
                      rootObj['dependencies'] = obj;
                    }
      					    fs.writeFile(silkedit.Constants.userRootPackageJsonPath, JSON.stringify(rootObj), (err) => {
      					      if (err) throw err

        						  silkedit.ProjectManager.open(pkgPath);
        						  // fixme: package.json is opened in another window (not new window opened above)
        						  // silkedit.ProjectManager.open(path.join(pkgPath, 'package.json'));
      					    });
      					  });
   					    });
   					  });
   					});
   				});
   			});
      });
		},
		"newline": () => {
			const editView = silkedit.App.activeTextEditView()
			if (editView != null) {
				editView.insertNewLine()
			}
		},
		"indent": () => {
			const editView = silkedit.App.activeTextEditView();
			if (editView != null) {
				editView.indent();
			}
		},
  	"outdent": () => {
			const editView = silkedit.App.activeTextEditView();
			if (editView != null) {
				editView.outdent();
			}
		},
		"select_next_tab": () => {
			const tabView = silkedit.App.activeTabView();
			if (tabView != null) {
				const currentIndex = tabView.currentIndex;
				if (currentIndex + 1 >= tabView.count) {
					tabView.currentIndex = 0;
				} else {
					tabView.currentIndex = currentIndex + 1;
				}
			}
		},
		"select_previous_tab": () => {
			const tabView = silkedit.App.activeTabView();
			if (tabView != null) {
				const currentIndex = tabView.currentIndex;
				if (currentIndex - 1 < 0) {
					tabView.currentIndex = tabView.count - 1;
				} else {
					tabView.currentIndex = currentIndex - 1;
				}
			}
		},
		"show_console": () => {
      const win = silkedit.App.activeWindow();
      if (win) {
        const view = win.console();
        if (view) {
          view.visible = true;
        }
      }
		},
		"hide_console": () => {
      const win = silkedit.App.activeWindow();
      if (win) {
        const view = win.console();
        if (view) {
          view.visible = false;
        }
      }
		}
	}
}
