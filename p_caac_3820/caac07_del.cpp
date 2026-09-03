/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      ZHOULI
Version:     3.0
Date:        2011-39-14 09:39:03
Description: 费用分摊规则修改
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac07_del)

int f_caac07_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int   doFlag = 0;
	int   fetchRowCount = 0;
	/* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac07("TCAAC07");

	// 数据库SQL操作字符串
	CString  sqlstr("");

	CDbCommand cmd_inq(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();

		for (int i = 0; i < count; i++)
		{
			//重置头文件,获得头文件中的默认值
			tcaac07.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据
			tcaac07.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tcaac07.Delete("WCE,SUB_BACKLOG_CODE,MAT_CODE,DEPT_CODE");
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}

