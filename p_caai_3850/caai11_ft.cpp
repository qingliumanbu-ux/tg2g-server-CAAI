/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    3.0
Date:		2020-04-09
Description:月成本分摊
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caai11_ft)
//-EP_SYSTEM_HEAD_END
int f_caai_allft(CString account_period, CString dept_code, CDbConnection * conn);
int f_caai11_ft(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int doFlag1 = 0;
	int fetchRowCount = 0;

	CString   dept_code = "";
	CString   account_period = "";
	CString   cost_center = "";
	CString   mat_code = "";
	CString   mat_name = "";


	CString  sqlstr("");

	CDbCommand cmd_inq(conn);

	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		account_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"].ToString().SubstringNE(0, 6);
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString();

		if (account_period.Trim() == "" || dept_code.Trim()=="")
		{
			strcpy(s.msg, "会计期为空或分厂为空，请选择!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		doFlag1 = f_caai_allft(account_period,dept_code, conn);

		if (doFlag1 != 0)
		{
			strcpy(s.msg, "函数调用失败!");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		else
		{
			sprintf(s.msg, "执行成功！");
		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		//Log::Trace("", "", "sqlstr={0}", sqlstr);
		//Log::Trace("", "", "str={0}", str);
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