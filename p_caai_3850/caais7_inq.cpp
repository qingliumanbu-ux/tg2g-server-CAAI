/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		
Version:    3.0
Date:		2020-7-28
Description:委外加工费查询
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caais7_inq)
//-EP_SYSTEM_HEAD_END
int f_caai_jg2(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_caais7_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;


	CString		account_period = " ";
	CString		mat_code = " ";
	CString		shift_group = " ";
	CString		dept_code = " ";
	CString		begin_time = " ";
	CString		end_time = " ";

	CModel tcaais7("TCAAIS7");


	CString  sqlstr("");


	CDbCommand cmd_inq(conn);

	try
	{
		tcaais7.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString().SubstringNE(0, 8);
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString().SubstringNE(0, 8);

		sqlstr = " select * from tcaais7  "
			" where 1=1 ";
		if (tcaais7["HEAT_NO"].ToString().Trim() != "")
			sqlstr = sqlstr + " and HEAT_NO like '%" + tcaais7["HEAT_NO"].ToString() + "%' ";
		if (tcaais7["ROLL_PLAN_NO"].ToString().Trim() != "")
			sqlstr = sqlstr + " and ROLL_PLAN_NO like '%" + tcaais7["ROLL_PLAN_NO"].ToString() + "%' ";
		if (tcaais7["SG_SIGN"].ToString().Trim() != "")
			sqlstr = sqlstr + " and SG_SIGN like '%" + tcaais7["SG_SIGN"].ToString() + "%' ";
		if (tcaais7["PLAN_NO"].ToString().Trim() != "")
			sqlstr = sqlstr + " and PLAN_NO like '%" + tcaais7["PLAN_NO"].ToString() + "%' ";
		if (tcaais7["OUT_UNIT_CODE"].ToString().Trim() != "")
			sqlstr = sqlstr + " and OUT_UNIT_CODE ='" + tcaais7["OUT_UNIT_CODE"].ToString() + "' ";
		if (tcaais7["SEND_FLAG"].ToString().Trim() == "1")
			sqlstr = sqlstr + " and SEND_FLAG = '1' ";		
		if (tcaais7["RECV_DEPT"].ToString().Trim() != "")
			sqlstr = sqlstr + " and RECV_DEPT = '" + tcaais7["RECV_DEPT"].ToString() + "' ";		
		if (end_time.Trim() != "")
			sqlstr = sqlstr + " and PROD_DATE <= '" + end_time + "' ";
		if (begin_time.Trim() != "")
			sqlstr = sqlstr + " and PROD_DATE >= '" + begin_time + "' ";
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();


		/*EIClass bcls_rec1;
		EIClass bcls_ret1;
		bcls_rec1.Tables[0].Columns.Add(DT_STRING, "MAT_NO");
		bcls_rec1.Tables[0].Columns.Add(DT_STRING, "PLAN_NO");
		bcls_rec1.Tables[0].Columns.Add(DT_DECIMAL, "WT");
		bcls_rec1.Tables[0].Rows.Add();
		bcls_rec1.Tables[0].Rows[0]["MAT_NO"] = "W1120B0345008";
		bcls_rec1.Tables[0].Rows[0]["PLAN_NO"] = "W1120B0345";
		bcls_rec1.Tables[0].Rows[0]["WT"] = 100;

		doFlag = f_caai_jg2(&bcls_rec1, &bcls_ret1, conn);
*/
		
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