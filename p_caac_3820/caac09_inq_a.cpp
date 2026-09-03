/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:根据工序代码、权重系数类别查询未设置权重系数的钢种规格
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac09_inq_a)
//-EP_SYSTEM_HEAD_END

int f_caac09_inq_a(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{

	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CModel tcaac09("TCAAC09");


	CString  sqlstr("");
	CString v_code = "";


	CDbCommand cmd_inq(conn);

	try
	{


		tcaac09.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句

			sqlstr = "	SELECT	MAT_THICK,MAT_WIDTH,SG_SIGN,@tcaac09.SUB_BACKLOG_CODE as  SUB_BACKLOG_CODE "
				" FROM tcaai04  t4";
			sqlstr = sqlstr + " WHERE NOT EXISTS (select 1 from tcaac09 t9 where t4.mat_thick = t9.mat_thick and t4.mat_width = t9.mat_width and t4.sg_sign =  t9.sg_sign and SUB_BACKLOG_CODE = @tcaac09.SUB_BACKLOG_CODE)	";
			sqlstr = sqlstr + " AND SUB_BACKLOG_CODE = @tcaac09.SUB_BACKLOG_CODE "
				" group by MAT_THICK,MAT_WIDTH,SG_SIGN"
				" order by MAT_THICK,MAT_WIDTH,SG_SIGN"
				;

			break;
		}

		Log::Trace("", __FUNCTION__, "sqlstr = 【{0}】", (const char*)sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tcaac09.SUB_BACKLOG_CODE", tcaac09["SUB_BACKLOG_CODE"]);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

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

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}