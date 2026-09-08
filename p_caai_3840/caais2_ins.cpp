/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:分摊摊销信息新增
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caais2_ins)
//-EP_SYSTEM_HEAD_END

int f_caais2_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	
	CModel tcaais2("TCAAIS2");

	CString		datetime = " ";
	CString  sqlstr("");
	

	CDbCommand cmd_inq(conn);
	
	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			//将对象字段重置为默认值
			tcaais2.Reset();

			// 获取前台传入参数
			tcaais2.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			if (tcaais2["MAT_CODE"].ToString().Trim()=="")
			{
				sprintf(s.msg, "物料编码不能为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if (tcaais2["STATS_PERIOD"].ToString().Trim() == "")
			{
				sprintf(s.msg, "统计期不能为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if (tcaais2["SUB_BACKLOG_CODE"].ToString().Trim() == "")
			{
				sprintf(s.msg, "工序不能为空!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}

			tcaais2["REC_CREATE_TIME"] = CDateTime::Now().ToString("yyyyMMddHHmmss");
			tcaais2["REC_CREATOR"] = s.userid;

			sqlstr = " select distinct code_line from tcaac14 where sub_backlog_code=@sub_backlog_code ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("sub_backlog_code", tcaais2["SUB_BACKLOG_CODE"]);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tcaais2["DEPT_CODE"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();

			sqlstr = " select MEASURE_MODE "
				" FROM TCAAC07 "
				" WHERE SUB_BACKLOG_CODE =@sub_backlog_code "
				" AND MAT_CODE = @mat_code "
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("sub_backlog_code", tcaais2["SUB_BACKLOG_CODE"]);
			cmd_inq.Parameters.Set("mat_code", tcaais2["MAT_CODE"]);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				tcaais2["DIVVY_TYPE"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();

			tcaais2["DATA_FROM"] = "I";

// DM8 适配 CHANGE-263:查询。空串搜索 DECODE 改为标准 CASE。
// 改写原因：空串搜索 DECODE(x,'',a,b) 改为标准 CASE WHEN x IS NULL OR x='' THEN a ELSE b,与 CHANGE-107 同理,不依赖空串/NULL 匹配的未记载语义；依据 DM 官方文档,DM8 尚未实测。
// 本共用分支面向 DM8,其他 DB_KIND 标签也会执行此 SQL;参数、结果列、条件与排序保持不变。
// 原 SQL（完整保留）：
			// sqlstr = "select  @data_from||decode (key_seq,'','00001',trim(to_char(to_number(nvl(key_seq,0))+1, '00000'))) from ("
				// " select max(substr(key_seq,length(@data_from)+1,6)) key_seq"
				// " from tcaais2"
				// " where 1=1"
				// " and data_from =@data_from"
				// " and sub_backlog_code = @sub_backlog_code"
				// " and stats_period = @stats_period"
				// ")"
				// ;
// DM8 SQL：
			sqlstr = "select  @data_from||CASE WHEN key_seq IS NULL OR key_seq = '' THEN '00001' ELSE trim(to_char(to_number(nvl(key_seq,0))+1, '00000')) END from ("
				" select max(substr(key_seq,length(@data_from)+1,6)) key_seq"
				" from tcaais2"
				" where 1=1"
				" and data_from =@data_from"
				" and sub_backlog_code = @sub_backlog_code"
				" and stats_period = @stats_period"
				")"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("sub_backlog_code", tcaais2["SUB_BACKLOG_CODE"].ToString());
			cmd_inq.Parameters.Set("stats_period", tcaais2["STATS_PERIOD"].ToString());
			cmd_inq.Parameters.Set("data_from", tcaais2["DATA_FROM"].ToString());
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tcaais2["KEY_SEQ"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();

			tcaais2.Insert();

		}

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