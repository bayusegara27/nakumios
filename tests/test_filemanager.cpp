/*
 * Unit tests for FileManager (NakumiFiles)
 *
 * Tests file listing, navigation, size formatting,
 * and directory operations.
 */

#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include "filemanager.h"

class TestFileManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testInitialPath();
    void testListDirectory();
    void testNavigateToDirectory();
    void testGoUp();
    void testGoBack();
    void testGoHome();
    void testCreateDirectory();
    void testDeleteFile();
    void testRoleNames();
    void testOpenDirectory();

private:
    QTemporaryDir *m_tempDir = nullptr;
};

void TestFileManager::initTestCase() {
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());

    /* Create test directory structure */
    QDir dir(m_tempDir->path());
    dir.mkdir("subdir1");
    dir.mkdir("subdir2");

    QFile f1(dir.filePath("file1.txt"));
    f1.open(QIODevice::WriteOnly);
    f1.write("Hello, World!");
    f1.close();

    QFile f2(dir.filePath("file2.cpp"));
    f2.open(QIODevice::WriteOnly);
    f2.write("int main() { return 0; }");
    f2.close();
}

void TestFileManager::cleanupTestCase() {
    delete m_tempDir;
}

void TestFileManager::testInitialPath() {
    FileManager fm;
    QCOMPARE(fm.currentPath(), QDir::homePath());
}

void TestFileManager::testListDirectory() {
    FileManager fm;
    fm.setCurrentPath(m_tempDir->path());
    /* Should have: subdir1, subdir2, file1.txt, file2.cpp = 4 entries */
    QCOMPARE(fm.rowCount(), 4);
}

void TestFileManager::testNavigateToDirectory() {
    FileManager fm;
    fm.setCurrentPath(m_tempDir->path());
    QString expected = m_tempDir->path();
    QCOMPARE(fm.currentPath(), expected);
}

void TestFileManager::testGoUp() {
    FileManager fm;
    fm.setCurrentPath(m_tempDir->path() + "/subdir1");
    fm.goUp();
    QCOMPARE(fm.currentPath(), m_tempDir->path());
}

void TestFileManager::testGoBack() {
    FileManager fm;
    fm.setCurrentPath(m_tempDir->path());
    fm.setCurrentPath(m_tempDir->path() + "/subdir1");
    QVERIFY(fm.canGoBack());
    fm.goBack();
    QCOMPARE(fm.currentPath(), m_tempDir->path());
}

void TestFileManager::testGoHome() {
    FileManager fm;
    fm.setCurrentPath(m_tempDir->path());
    fm.goHome();
    QCOMPARE(fm.currentPath(), QDir::homePath());
}

void TestFileManager::testCreateDirectory() {
    FileManager fm;
    fm.setCurrentPath(m_tempDir->path());
    int before = fm.rowCount();
    fm.createDirectory("newdir");
    QCOMPARE(fm.rowCount(), before + 1);

    /* Verify the directory exists */
    QVERIFY(QDir(m_tempDir->path() + "/newdir").exists());
}

void TestFileManager::testDeleteFile() {
    FileManager fm;
    fm.setCurrentPath(m_tempDir->path());

    /* Create a file to delete */
    QFile tempFile(m_tempDir->path() + "/deleteme.txt");
    tempFile.open(QIODevice::WriteOnly);
    tempFile.write("delete this");
    tempFile.close();

    fm.refresh();
    int before = fm.rowCount();

    /* Find the index of deleteme.txt */
    int delIdx = -1;
    for (int i = 0; i < fm.rowCount(); ++i) {
        QModelIndex idx = fm.index(i);
        if (fm.data(idx, FileManager::NameRole).toString() == "deleteme.txt") {
            delIdx = i;
            break;
        }
    }
    QVERIFY(delIdx >= 0);
    fm.deleteFile(delIdx);
    QCOMPARE(fm.rowCount(), before - 1);
}

void TestFileManager::testRoleNames() {
    FileManager fm;
    auto roles = fm.roleNames();
    QVERIFY(roles.contains(FileManager::NameRole));
    QVERIFY(roles.contains(FileManager::PathRole));
    QVERIFY(roles.contains(FileManager::SizeRole));
    QVERIFY(roles.contains(FileManager::IsDirRole));
}

void TestFileManager::testOpenDirectory() {
    FileManager fm;
    fm.setCurrentPath(m_tempDir->path());

    /* Find subdir1 index */
    int subdirIdx = -1;
    for (int i = 0; i < fm.rowCount(); ++i) {
        QModelIndex idx = fm.index(i);
        if (fm.data(idx, FileManager::NameRole).toString() == "subdir1") {
            subdirIdx = i;
            break;
        }
    }
    QVERIFY(subdirIdx >= 0);
    fm.open(subdirIdx);
    QCOMPARE(fm.currentPath(), m_tempDir->path() + "/subdir1");
}

QTEST_MAIN(TestFileManager)
#include "test_filemanager.moc"
