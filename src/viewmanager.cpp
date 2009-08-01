/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "viewmanager.h"
#include "statetableview.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "gridview.h"
#include "defines.h"

ViewManager::ViewManager(DwarfModel *dm, DwarfModelProxy *proxy, QWidget *parent)
	: QTabWidget(parent)
	, m_model(dm)
	, m_proxy(proxy)
{
	m_proxy->setSourceModel(m_model);

	QPushButton *btn = new QPushButton("Add View", this);
	setCornerWidget(btn);
	
	reload_views();
	
	
	//tabBar()->setTabButton(1, QTabBar::RightSide, btn);
}

void ViewManager::reload_views() {
	// make sure the required directories are in place
	// TODO make directory locations configurable
	QDir cur = QDir::current();
	if (!cur.exists("etc/views")) {
		QMessageBox::warning(this, tr("Missing Directory"),
			tr("Could not fine the 'views' directory under 'etc'"));
		return;
	}
	if (!cur.exists("etc/sets")) {
		QMessageBox::warning(this, tr("Missing Directory"),
			tr("Could not fine the 'sets' directory under 'etc'"));
		return;
	}

	// goodbye old views!
	foreach(GridView *v, m_views) {
		v->deleteLater();
	}
	m_views.clear();

	QDir views = QDir(QDir::currentPath() + "/etc/views");
	QDir sets = QDir(QDir::currentPath() + "/etc/sets");
	
	QStringList view_files = views.entryList(QDir::Files | QDir::Readable, QDir::Time);
	foreach(QString filename, view_files) {
		if (filename.endsWith(".ini")) {
			LOGD << "found view file" << views.filePath(filename);
			GridView *v = GridView::from_file(views.filePath(filename), sets, this);
			if (v)
				m_views << v;
			if (v->is_active())
				add_tab_for_gridview(v);
		}
	}
	connect(tabBar(), SIGNAL(currentChanged(int)), this, SLOT(setCurrentIndex(int)));
	m_model->set_grid_view(m_views[0]);
	
	
	// add view to file
	// set main view to default
	// make a tab out of each activated view + the default
}

void ViewManager::setCurrentIndex(int idx) {
	foreach(GridView *v, m_views) {
		if (v->name() == tabBar()->tabText(idx)) {
			m_model->set_grid_view(v);
			m_model->build_rows();
			break;
		}
	}
}

int ViewManager::add_tab_for_gridview(GridView *v) {
	StateTableView *stv = new StateTableView(this);
	stv->set_model(m_model, m_proxy);
	
	connect(this, SIGNAL(color_changed(const QString &, const QColor &)),
			stv, SLOT(color_changed(const QString &, const QColor &)));
	connect(stv, SIGNAL(customContextMenuRequested(const QPoint &)), 
			this, SLOT(draw_grid_context_menu(const QPoint &)));

	return addTab(stv, v->name());
}

void ViewManager::draw_grid_context_menu(const QPoint &p) {
	StateTableView *stv = qobject_cast<StateTableView*>(currentWidget());
	QModelIndex idx = stv->indexAt(p);
	if (!idx.isValid() || idx.column() != 0  || idx.data(DwarfModel::DR_IS_AGGREGATE).toBool())
		return;

	QMenu m(this);
	m.setTitle(tr("Dwarf Options"));
	m.addAction(tr("Set Nickname..."), this, SLOT(set_nickname()));
	//m.addAction(tr("View Details..."), this, "add_custom_profession()");
	m.addSeparator();

	QMenu sub(&m);
	sub.setTitle(tr("Custom Professions"));
	sub.addAction(tr("New custom profession from this dwarf..."), this, SLOT(add_custom_profession()));
	sub.addAction(tr("Reset to default profession"), this, SLOT(reset_custom_profession()));
	sub.addSeparator();
	
	/*foreach(CustomProfession *cp, m_custom_professions) {
		sub.addAction(cp->get_name(), this, SLOT(apply_custom_profession()));
	}
	m.addMenu(&sub);
	*/
	
	m.exec(stv->viewport()->mapToGlobal(p));
}

void ViewManager::color_changed(const QString &key, const QColor &c) {
	/*FIXME
	UberDelegate *d = m_stv->get_delegate();
	if (key == "cursor")
		d->color_cursor = c;
	else if (key == "dirty_border")
		d->color_dirty_border = c;
	else if (key == "active_labor")
		d->color_active_labor = c;
	else if (key == "active_group")
		d->color_active_group = c;
	else if (key == "inactive_group")
		d->color_inactive_group = c;
	else if (key == "partial_group")
		d->color_partial_group = c;
	else if (key == "guides")
		d->color_guides = c;
	else if (key == "border")
		d->color_border = c;
	else
		qWarning() << "some color changed and I don't know what it is.";
	*/
}