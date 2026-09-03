/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:能源采集信息查询
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caai12_inq)
//-EP_SYSTEM_HEAD_END

int f_caai12_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	

	CString		begin_time				=	" ";
	CString		end_time				=	" ";

	CString  sqlstr("");
	CString flag = "0";
	CModel tcaaia12("TCAAIA12");

	CDbCommand cmd_inq(conn);
	
	try
	{
		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString().SubstringNE(0, 8);
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString().SubstringNE(0, 8);
		tcaaia12.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		if (bcls_rec->Tables[0].Columns.Contains("FLAG"))
		{
			flag = bcls_rec->Tables[0].Rows[0]["FLAG"].ToString();
		}

		if (flag != "1")
		{

			sqlstr = " SELECT * FROM TCAAIA12"
				" WHERE 1=1"
				;
			if (tcaaia12["MAT_NAME"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  mat_NAME   LIKE '%'|| @mat_name||'%'";
			if (tcaaia12["MAT_CODE"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  mat_code    = @mat_code";
			if (tcaaia12["COST_CENTER"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  COST_CENTER    = @cost_center";
			if (tcaaia12["SHIFT_NO"].ToString().Trim() != "")      sqlstr = sqlstr + " AND  shift_no = @shift_no";
			if (tcaaia12["BUSI_TYPE"].ToString().Trim() != "")      sqlstr = sqlstr + " AND  BUSI_TYPE = @busi_type";
			if (tcaaia12["PLAN_NO"].ToString().Trim() != "")      sqlstr = sqlstr + " AND  PLAN_NO LIKE @plan_no||'%'";
			if (tcaaia12["DEPT_CODE"].ToString().Trim() != "")        sqlstr = sqlstr + " AND  dept_code = @dept_code";
			sqlstr = sqlstr +" AND PROD_DATE <=@end_time"
				" and PROD_DATE >=@begin_time"
				;
			sqlstr = sqlstr + " order by REC_TIME,DEV_POS,MAT_NAME";
		}
		else
		{
			sqlstr = " SELECT * FROM ("
				" SELECT * FROM TCAAIA12"
				" WHERE 1=1"
				" and BUSI_TYPE in ('TJ','YJ','ZH')"
				" and PROD_DATE >=@begin_time"
				" AND PROD_DATE <=@end_time"
				" union all"
				" SELECT * FROM TCAAIA12"
				" WHERE 1=1"
				" AND PLAN_NO IN (SELECT DECODE(DEPT_CODE,'S',PONO,PLAN_NO) FROM TCAAIA14 WHERE STATS_PERIOD >=@begin_time AND STATS_PERIOD <=@end_time )"
				" and BUSI_TYPE ='SC'"
				")"
				" WHERE 1=1"
				;
			if (tcaaia12["MAT_NAME"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  mat_NAME   LIKE '%'|| @mat_name||'%'";
			if (tcaaia12["MAT_CODE"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  mat_code    = @mat_code";
			if (tcaaia12["COST_CENTER"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  COST_CENTER    = @cost_center";
			if (tcaaia12["SHIFT_NO"].ToString().Trim() != "")      sqlstr = sqlstr + " AND  shift_no = @shift_no";
			if (tcaaia12["BUSI_TYPE"].ToString().Trim() != "")      sqlstr = sqlstr + " AND  BUSI_TYPE = @busi_type";
			if (tcaaia12["PLAN_NO"].ToString().Trim() != "")      sqlstr = sqlstr + " AND  PLAN_NO LIKE @plan_no||'%'";
			if (tcaaia12["DEPT_CODE"].ToString().Trim() != "")        sqlstr = sqlstr + " AND  dept_code = @dept_code";
			sqlstr = sqlstr + " order by REC_TIME,DEV_POS,MAT_NAME";

		}

		
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "sqlstr=[{0}]  ", sqlstr);
		cmd_inq.Parameters.Set("begin_time",begin_time);
		cmd_inq.Parameters.Set("end_time",end_time);
		cmd_inq.Parameters.Set("mat_code", tcaaia12["MAT_CODE"].ToString());
		cmd_inq.Parameters.Set("mat_name", tcaaia12["MAT_NAME"].ToString());
		cmd_inq.Parameters.Set("cost_center", tcaaia12["COST_CENTER"].ToString());
		cmd_inq.Parameters.Set("shift_no", tcaaia12["SHIFT_NO"].ToString());
		Log::Trace("", __FUNCTION__, "shift_no=[{0}]  ", tcaaia12["SHIFT_NO"].ToString().Trim());
		cmd_inq.Parameters.Set("dept_code", tcaaia12["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("busi_type", tcaaia12["BUSI_TYPE"].ToString());
		cmd_inq.Parameters.Set("plan_no", tcaaia12["PLAN_NO"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close(); 	

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		 
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
	
	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}