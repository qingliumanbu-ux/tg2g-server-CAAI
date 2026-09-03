/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:分摊摊销信息修改
**************************************************/
//框架用头文件
#include "stdafx.h"

//业务用头文件




// service入口
BM2F_ENTERACE(caais1_upd)
//-EP_SYSTEM_HEAD_END

int f_caais1_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;


	CString		datetime = " ";
	CModel tcaaia1("TCAAIA1");


	CString  sqlstr("");


	CDbCommand cmd_inq(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaaia1.Reset();

			tcaaia1.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tcaaia1["REC_REVISOR"] = s.userid;   //记录修改责任者
			tcaaia1["REC_REVISE_TIME"] = datetime;   //记录修改时刻

			//修改一条记录
			tcaaia1.Update("*", "DEPT_CODE, STATS_PERIOD, COST_CENTER, RULE_TYPE, DIVVY_TYPE, MAT_CODE,SUB_BACKLOG_CODE, EQU_NO, PROD_DATE, PROD_SHIFT_GROUP, PROD_SHIFT_NO");

		}

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
