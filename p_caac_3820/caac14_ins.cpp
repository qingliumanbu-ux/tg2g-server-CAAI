/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 物料工序使用配置新增
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac14_ins)
int f_caac14_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	int i =0;

	// 定义表的实体对象
	CModel tcaac14("TCAAC14");


	CDbCommand cmd_inq(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		// 传入块中第一个表的行数
		int rowCount = bcls_rec->Tables[0].Rows.get_Count();
		for(int i=0;  i < rowCount;  i++)
		{
			//将对象字段重置为默认值
			tcaac14.Reset();

			// 获取前台传入参数
			tcaac14.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tcaac14["REC_CREATOR"]     = s.userid;
			tcaac14["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");

			// 执行新增,失败抛出异常
			sqlstr = CString("tcaac14.Insert()");
			tcaac14.Insert(); 
		}


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}

