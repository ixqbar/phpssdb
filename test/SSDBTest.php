<?php

class SSDBTest extends PHPUnit_Framework_TestCase {

    protected $ssdb_handle;

    protected function setUp(){
        $this->ssdb_handle = new SSDB();
        $this->ssdb_handle->connect('127.0.0.1', 8888);
        //optional
        $this->ssdb_handle->option(SSDB::OPT_PREFIX, 'test_');
    }

    public function testAuth() {
        $this->assertTrue($this->ssdb_handle->auth('xingqiba'));
    }

    public function testDel() {
        $this->assertTrue($this->ssdb_handle->del('name'));
    }

    public function testGetNull() {
        $this->assertNull($this->ssdb_handle->get('name'));
    }

    public function testSet() {
        $this->assertTrue($this->ssdb_handle->set('name', 'xingqiba'));
    }

    public function testGet() {
        $this->assertEquals('xingqiba', $this->ssdb_handle->get('name'));
    }

    public function testSetnx() {
        $this->assertFalse($this->ssdb_handle->setnx('name', 'xingqiba'));
        $this->assertTrue($this->ssdb_handle->setnx('location', 'shanghai'));
        $this->assertTrue($this->ssdb_handle->del('location'));
    }

    public function testSetExpire() {
        $this->assertTrue($this->ssdb_handle->set('name', 'xingqiba', 3));
        sleep(4);
        $this->assertEmpty($this->ssdb_handle->get('name'));
    }

    public function testExpire() {
        $this->assertTrue($this->ssdb_handle->set('name', 'xingqiba'));
        $this->assertTrue($this->ssdb_handle->expire('name', 3));
        sleep(4);
        $this->assertEmpty($this->ssdb_handle->get('name'));
    }

    public function testGetSet() {
        $this->assertTrue($this->ssdb_handle->del('blog'));
        $this->assertEmpty($this->ssdb_handle->getset('blog', 'http://xingqiba.sinaapp.com/'));
        $this->assertEquals('http://xingqiba.sinaapp.com/', $this->ssdb_handle->get('blog'));
        $this->assertEquals('http://xingqiba.sinaapp.com/', $this->ssdb_handle->getset('blog', "http://linjujia.cn/"));
        $this->assertEquals('http://linjujia.cn/', $this->ssdb_handle->get('blog'));
    }

    public function testTtl() {
        $this->assertTrue($this->ssdb_handle->set('name', 'xingqiba', 3));
        $ttl = $this->ssdb_handle->ttl('name');
        $this->assertLessThanOrEqual(3, $ttl);
        $this->assertGreaterThan(0, $ttl);
    }

    public function testIncr() {
        $this->assertTrue($this->ssdb_handle->del('hits'));
        $this->assertEquals(1, $this->ssdb_handle->incr('hits'));
        $this->assertEquals(11, $this->ssdb_handle->incr('hits', 10));
    }

    public function testExists() {
        $this->assertTrue($this->ssdb_handle->exists('hits'));
        $this->assertFalse($this->ssdb_handle->exists('weibo'));
    }

    public function testSetbit() {
        $this->assertTrue($this->ssdb_handle->del('bit_test'));
        $this->assertEquals(0, $this->ssdb_handle->setbit('bit_test', 0, 1));
        $this->assertEquals(1, $this->ssdb_handle->setbit('bit_test', 0, 0));
        $this->assertEquals(0, $this->ssdb_handle->setbit('bit_test', 0, 1));
        $this->assertEquals(0, $this->ssdb_handle->setbit('bit_test', 1, 1));
    }

    public function testGetbit() {
        $this->assertEquals(1, $this->ssdb_handle->getbit('bit_test', 0));
        $this->assertEquals(1, $this->ssdb_handle->getbit('bit_test', 1));
    }

    public function testCountbit() {
        $this->assertEquals(2, $this->ssdb_handle->countbit('bit_test'));
        $this->assertEquals(1, $this->ssdb_handle->setbit('bit_test', 0, 0));
        $this->assertEquals(0, $this->ssdb_handle->setbit('bit_test', 9, 1));
        $this->assertEquals(0, $this->ssdb_handle->setbit('bit_test', 10, 1));
        $this->assertEquals(3, $this->ssdb_handle->countbit('bit_test'));
        $this->assertEquals(0, $this->ssdb_handle->countbit('bit_test', 9));
    }

    public function testSubstr() {
        $this->assertTrue($this->ssdb_handle->set('name', 'xingqiba'));
        $this->assertEquals("xingqiba", $this->ssdb_handle->substr('name',0));
        $this->assertEquals("xing", $this->ssdb_handle->substr('name',0, 4));
        $this->assertEquals("ba", $this->ssdb_handle->substr('name', 6));
    }

    public function testStrlen() {
        $this->assertTrue($this->ssdb_handle->set('name', 'xingqiba'));
        $this->assertEquals(strlen("xingqiba"), $this->ssdb_handle->strlen('name'));
    }


}